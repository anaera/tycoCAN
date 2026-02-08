#include "../_hard/canDrive.h"
#include "../_can/CAN.h"
#include "../_hard/initChip.h"
#include "../_hard/TMRx.h"
#include  "../_epp/epp.h"

#include "debug.h"

#define SET_GAP() 	{TMR0 = _TMR0; tmr.gapFlag = 1;}
#define CLR_GAP() 	{tmr.gapFlag = 0;}
#define EXIT_GAP    !tmr.gapFlag

void setError(uint8_t code);
void canSend(void);


uint8_t actStep;
stateDriver_t state;
controlDrive_t engine;
constant_t constant;

extern __eeprom ee_t _ee;
extern packCAN_t txCAN, rxCAN;
volatile tmr_t tmr;

inline void setGapTMR0(void)
{
	tmr.gapFlag = 1;
	TMR0 = _TMR0;
}

inline bool getGapTMR0(void)
{
	return tmr.gapFlag;
}

inline void clrGapTMR0(void)
{
	tmr.gapFlag = 0;
}

bool sys_flag(void)
{
	if (tmr.sysFlag) {
		tmr.sysFlag = 0;
		return true;
	}
	return false;
}

bool app_flag(void)
{
	if (tmr.appFlag) {
		tmr.appFlag = 0;
		return true;
	}
	return false;
}

bool cyc_flag(void)
{
	if (tmr.cycFlag) {
		tmr.cycFlag = 0;
		return true;
	}
	return false;
}

uint8_t tmrChkFlag(uint8_t mask)
{
	if (tmr.allFlags & mask) {
		tmr.allFlags ^= mask;
		return true;
	}
	return false;
}

bool tmrGetFlag(uint8_t mask)
{
	return(tmr.allFlags & mask);
}

void tmrClrFlag(uint8_t mask)
{
	tmr.allFlags &= (~mask);
}

uint8_t updByteCRC(uint8_t crc, uint8_t byte)
{
	crc = crc ^ byte;
	for (uint8_t i = 0; i < 8; i++) {
		crc = crc & 0x80 ? (uint8_t) (crc << 1) ^ constant.crcMask : (uint8_t) (crc << 1);
	}
	return crc;
}

void initCAN(uint8_t speedUART)
{
	initUART(speedUART);
	//загрузка констант uCanDrive
	constant.rcvMask = _ee._bit_mask; //маска бита выборки данных у нас она 0x10
	constant.crcMask = _ee._crc_poly; //полином CRC
	constant.gapTime = _ee._pack_gap;
	state.flags = 0;
	return;
}

void initNewCycle(void)
{
	engine.crcByte = 0;
	engine.cntPack = 0;
	engine.ptrPack = 0;
	engine.lenPack = CAN_LEN_HEADER;
	engine.bitMask = 0x80;
}


//переход в режим приема (выключаем прерывание)

inline void setStateReceive(void)
{

	SET_GAP(); //setGapTMR0(); защищаем прием
	//чтоб за время до полного приема байта в RCREG UART не сбросилось состояние RECV
	actStep = RECV_STEP;
}
//переход в режим ожиддания 

void setStateIdle(void)
{
	//задерживаем синхронный переход в режим передачи на время gapTime
	//для контролия линмм RxD включаем прерывание, позволяя другим занять шину
	SET_GAP(); //setGapTMR0(); защищаем переход в ожидании
	actStep = IDLE_STEP; //ждем приема или передачи	
	initNewCycle();

#ifdef INT_RA2
	INTCONbits.INTF = 0; //сбрасываем флаг
	INTCONbits.INTE = 1; //прерывание на прием активно
#else
	INTCONbits.IOCIF = 0;
	IOCAFbits.IOCAF3 = 0;
	INTCONbits.IOCIE = 1;

#endif
}

void setStateSend(void)
{
	//еще раз проверяем уже с запрещенным прерыванием di();
	if (actStep == IDLE_STEP) {
		SET_GAP(); //setGapTMR0(); защищаем передачу
		actStep = SEND_STEP;
#ifdef INT_RA2
		INTCONbits.INTE = 0; //запрещаем прерывание от INT0	
		INTCONbits.INTF = 0; //сбрасываем флаг 
#else
		INTCONbits.IOCIE = 0;
		INTCONbits.IOCIF = 0;
		IOCAFbits.IOCAF3 = 0;

#endif
		//TXREG = engine.tmpByte; //первый пошел (передача)	
		engine.sndByte = txCAN.raw[0]; //bufferTx[0];
		state.sndBit = (engine.sndByte & engine.bitMask) ? 1 : 0;
		TXREG = state.sndBit ? SEND_BYTE_HI : SEND_BYTE_LO;
		state.sndCounter--;
		return; //пошла передача
	} else {
		return; //не успели, уже читают
	}
}

void checkAndActSendPack(void)
{
	if (state.sndIsFull) {
		if (state.sndCounter) //исчерпано кол-во повторов текущего пакета?
		{
			//setStateSend----------------------------------
			if (actStep == IDLE_STEP) {
				di();
				setStateSend(); //переход в режим передачи или нет
				ei();
			}
			//setStateSend----------------------------------
			return;
		} else {
			state.sndIsFull = 0; //сбрасываем флаг
			setError(eTxD);
		}
	} else {
		canSend(); // формируем новый пакет на передачу (если есть данные)
	}
}

//машина состояний и разрешения коллизий протокола физического уровня 

void stateEngine(void)
{
	if (EXIT_GAP) { //if (!tmr.gapFlag) { //getGapTMR0() == 0
		//Завершение активной фазы приема/передачи пакета
		//только после завершения защитного интервала из любой ативной фазы переходим в состояние IDLE
		// и тольклько в следующем такте переходим в режим следующей активной фазы SEND
		// передача - синхронный переход по наличию флага готовности данных служб на отправку
		// прием - асинхронный по обнаружению спадающего фронта на входе (активация прерыванием)
		switch (actStep) {
		case RECV_STEP:
		case SKIP_STEP:
		case SEND_STEP:

			//инициализация перехода в режим ожидания для приема/передачи
			setStateIdle();
			return;
		case IDLE_STEP:
		{
			//через интервал gapTime контроль следующей готовности пакета на передачу

			checkAndActSendPack();
		}
		}
	}
}

inline void packActEngine(void)
{
	SET_GAP(); //setGapTMR0(); устанавливаем таймаут ожидания следующего байте
	//RA1_SetLow();
	// Чтение и обработка принятого бита (принятый байт) (Очищаем буфер приема RCREG!)
	state.rcvBit = (RCREG & constant.rcvMask) ? 1 : 0;
	// Проверяем наличие аппаратных ошибок в приёмнике USART и сбрасываем их
	if (RCSTAbits.FERR | RCSTAbits.OERR) {
		RCSTAbits.CREN = 0;
		NOP();
		RCSTAbits.CREN = 1;
		setError(eUART); //  0x01  ошибока UART
		actStep = SKIP_STEP;
		return;
	}

	// Если ранее была какая либо ошибка т.е. SKIP_STEP
	// Сбрасываем все последующие байты принимаемого пакета до  таймаута пакета
	// gapTime - время таймаута,которое считается в TMR0
	if (actStep == SKIP_STEP) return;
	// Предидущий принятый пакет уже обработан?
	if (state.rcvIsFull) { //Нет
		setError(eRxD); // 0x10  ошибока переполнения буфера приема RxD 
		actStep = SKIP_STEP;
		return;
	}

	//Контроль ошибки передачи и Арбитраж
	if (actStep == SEND_STEP) {
		if (state.rcvBit != state.sndBit) {
			// Обработка бита приоритета адреса пакета (переключения передачи/приёма)		
			if (engine.ptrPack < 12) {
				if (!state.rcvBit) {
					setError(eARBIT); // // 0x80  проигран арбитраж
					actStep = RECV_STEP; // уступили
				}
			} else {
				setError(eSEND); // 0x02  ошибока арбитража байта
				actStep = SKIP_STEP;
				return;
			}
		}
	}
	// Все нормально, можно обрабатывать валидный бит пакета (кладем в текущий байт)
	if (state.rcvBit) {
		engine.rcvByte |= engine.bitMask;
	} else {
		engine.rcvByte &= ~engine.bitMask;
	}
	// Подготовка к приему/передаче следующему бита
	engine.bitMask >>= 1;
	engine.ptrPack++;
	if (!engine.bitMask) { // Все биты байта принят/передан?
		//Переход к следующему байту
		engine.bitMask = 0x80;
		//RA1_Toggle();
		// Обновление контрольной суммы с учетом принятого байта
		engine.crcByte = engine.crcByte & 0x80 ? (uint8_t) (engine.crcByte << 1) ^ constant.crcMask : (uint8_t) (engine.crcByte << 1);
		// Завершился приём заголовка пакета?	
		if (engine.cntPack == 1) {
			engine.lenData = engine.rcvByte & 0x0F; //Выделяем длину пакета данных
			if (engine.lenData > 8) {
				setError(eLEN); // 0x04  ошибока размера пакета
				actStep = SKIP_STEP;
				return;
			}
			engine.lenPack += engine.lenData; // engine.lenPack уже содержит длину заголовка +1(crc)
		}
		// Полностью принято нужное количество байтов?
		if (engine.cntPack >= engine.lenPack) {
			// Сюда заходим только когда == и тут проверяем CRC
			NOP();
			//			RA0_SetLow();
			if (engine.crcByte == 0) {
				if (actStep == RECV_STEP) {
					state.rcvIsFull = 1;
					clrGapTMR0(); //сбрасываем защитный интервал
					//tmr.gapFlag = 0;
					return;
				}
				if (actStep == SEND_STEP) {
					state.sndIsFull = 0;
					// Если включён режим эха то добавляем готовность приема
					//if (state.echoTX) {
					//	state.rcvIsFull = 1;
					//}
					return;
				}
			} else {
				setError(eCRC); // 0x08  ошибока CRC
				actStep = SKIP_STEP;
				return;
			}
		}
		// Продолжаем прием/передачу пакета данных
		rxCAN.raw[engine.cntPack] = engine.rcvByte;
		//		NOP();
		engine.cntPack++; //  следующий байт 
		if (actStep == SEND_STEP) { //  передаем ? 
			engine.sndByte = txCAN.raw[engine.cntPack]; //да, передаем еще
		}
	}

	if (actStep == SEND_STEP) { // Передача очередного бита
		state.sndBit = (engine.sndByte & engine.bitMask) ? 1 : 0;
		TXREG = (state.sndBit) ? SEND_BYTE_HI : SEND_BYTE_LO;
		SET_GAP(); //setGapTMR0(); защищаем передачу
	}
}

uint8_t isRxReady(void)
{
	return(state.rcvIsFull == 1);
}

uint8_t isTxReady(void)
{
	return(state.sndIsFull == 0);
}

void setEcho(void)
{
	state.echoTX = 1;
}

void clrEcho(void)
{
	state.echoTX = 0;
}

void startTx(void)
{
	state.sndCounter = TxRETRY;
	state.sndIsFull = 1;
}

void clearTx(void)
{
	state.sndCounter = 0;
	state.sndIsFull = 0;
}

void clearRx(void)
{
	state.rcvIsFull = 0;
}
