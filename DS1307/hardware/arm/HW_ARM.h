void DS1307::begin()
{
	_use_hw = false;
	if ((_sda_pin == SDA) and (_scl_pin == SCL))
	{
		_use_hw = true;
		twi = TWI1;
		pmc_enable_periph_clk(WIRE_INTERFACE_ID);
		PIO_Configure(g_APinDescription[PIN_WIRE_SDA].pPort, g_APinDescription[PIN_WIRE_SDA].ulPinType, g_APinDescription[PIN_WIRE_SDA].ulPin, g_APinDescription[PIN_WIRE_SDA].ulPinConfiguration);
		PIO_Configure(g_APinDescription[PIN_WIRE_SCL].pPort, g_APinDescription[PIN_WIRE_SCL].ulPinType, g_APinDescription[PIN_WIRE_SCL].ulPin, g_APinDescription[PIN_WIRE_SCL].ulPinConfiguration);
		NVIC_DisableIRQ(TWI1_IRQn);
		NVIC_ClearPendingIRQ(TWI1_IRQn);
		NVIC_SetPriority(TWI1_IRQn, 0);
		NVIC_EnableIRQ(TWI1_IRQn);

	}
	else if ((_sda_pin == SDA1) and (_scl_pin == SCL1))
	{
		_use_hw = true;
		twi = TWI0;
		pmc_enable_periph_clk(WIRE1_INTERFACE_ID);
		PIO_Configure(g_APinDescription[PIN_WIRE1_SDA].pPort, g_APinDescription[PIN_WIRE1_SDA].ulPinType, g_APinDescription[PIN_WIRE1_SDA].ulPin, g_APinDescription[PIN_WIRE1_SDA].ulPinConfiguration);
		PIO_Configure(g_APinDescription[PIN_WIRE1_SCL].pPort, g_APinDescription[PIN_WIRE1_SCL].ulPinType, g_APinDescription[PIN_WIRE1_SCL].ulPin, g_APinDescription[PIN_WIRE1_SCL].ulPinConfiguration);
		NVIC_DisableIRQ(TWI0_IRQn);
		NVIC_ClearPendingIRQ(TWI0_IRQn);
		NVIC_SetPriority(TWI0_IRQn, 0);
		NVIC_EnableIRQ(TWI0_IRQn);
	}

	if (_use_hw)
	{
		// activate internal pullups for twi.
		digitalWrite(SDA, 1);
		digitalWrite(SCL, 1);

		// Reset the TWI
		twi->TWI_CR = TWI_CR_SWRST;
		// TWI Slave Mode Disabled, TWI Master Mode Disabled.
		twi->TWI_CR = TWI_CR_SVDIS;
		twi->TWI_CR = TWI_CR_MSDIS;
		// Set TWI Speed
		twi->TWI_CWGR = (TWI_DIV << 16) | (TWI_SPEED << 8) | TWI_SPEED;
		// Set master mode
		twi->TWI_CR = TWI_CR_MSEN;
	}
	else
	{
		pinMode(_scl_pin, OUTPUT);
	}
}

void DS1307::_burstRead()
{
	if (_use_hw)
	{
		// Set slave address and number of internal address bytes.
		twi->TWI_MMR = (1 << 8) | TWI_MMR_MREAD | (DS1307_ADDR << 16);
		// Set internal address bytes
		twi->TWI_IADR = 0;
		// Send START condition
		twi->TWI_CR = TWI_CR_START;

		for (int i=0; i<6; i++)
		{
			while ((twi->TWI_SR & TWI_SR_RXRDY) != TWI_SR_RXRDY) {};
			_burstArray[i] = twi->TWI_RHR;
		}

		twi->TWI_CR = TWI_CR_STOP;
		while ((twi->TWI_SR & TWI_SR_RXRDY) != TWI_SR_RXRDY) {};
		_burstArray[6] = twi->TWI_RHR;
		while ((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {};
	}
	else
	{
		_sendStart(DS1307_ADDR_W);
		_waitForAck();
		_writeByte(0);
		_waitForAck();
		_sendStart(DS1307_ADDR_R);
		_waitForAck();

		for (int i=0; i<7; i++)
		{
			_burstArray[i] = _readByte();
			if (i<6)
				_sendAck();
			else
				_sendNack();
		}
		_sendStop();
	}
}

uint8_t DS1307::_readRegister(uint8_t reg)
{
	uint8_t	readValue=0;

	if (_use_hw)
	{
		// Set slave address and number of internal address bytes.
		twi->TWI_MMR = (1 << 8) | TWI_MMR_MREAD | (DS1307_ADDR << 16);
		// Set internal address bytes
		twi->TWI_IADR = reg;
		// Send START and STOP condition to read a single byte
		twi->TWI_CR = TWI_CR_START | TWI_CR_STOP;
		while ((twi->TWI_SR & TWI_SR_RXRDY) != TWI_SR_RXRDY) {};
		readValue = twi->TWI_RHR;
		while ((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {};
	}
	else
	{
		_sendStart(DS1307_ADDR_W);
		_waitForAck();
		_writeByte(reg);
		_waitForAck();
		_sendStart(DS1307_ADDR_R);
		_waitForAck();
		readValue = _readByte();
		_sendNack();
		_sendStop();
	}
	return readValue;
}

void DS1307::_writeRegister(uint8_t reg, uint8_t value)
{
	if (_use_hw)
	{
		// Set slave address and number of internal address bytes.
		twi->TWI_MMR = (1 << 8) | (DS1307_ADDR << 16);
		// Set internal address bytes
		twi->TWI_IADR = reg;
		// Send a single byte to start transfer
		twi->TWI_THR = value;
		while ((twi->TWI_SR & TWI_SR_TXRDY) != TWI_SR_TXRDY) {};
		// Send STOP condition
		twi->TWI_CR = TWI_CR_STOP;
		while ((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {};
	}
	else
	{
		_sendStart(DS1307_ADDR_W);
		_waitForAck();
		_writeByte(reg);
		_waitForAck();
		_writeByte(value);
		_waitForAck();
		_sendStop();
	}
}

void DS1307::writeBuffer(DS1307_RAM r)
{
	if (_use_hw)
	{
		// Set slave address and number of internal address bytes.
		twi->TWI_MMR = (1 << 8) | (DS1307_ADDR << 16);
		// Set internal address bytes
		twi->TWI_IADR = 8;
		// Send the array of bytes
		for (int i=0; i<56; i++)
		{
			twi->TWI_THR = r.cell[i];
			while ((twi->TWI_SR & TWI_SR_TXRDY) != TWI_SR_TXRDY) {};
		}
		// Send STOP condition
		twi->TWI_CR = TWI_CR_STOP;
		while ((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {};
	}
	else
	{
		_sendStart(DS1307_ADDR_W);
		_waitForAck();
		_writeByte(8);
		_waitForAck();

		for (int i=0; i<56; i++)
		{
			_writeByte(r.cell[i]);
			_waitForAck();
		}

		_sendStop();
	}
}

DS1307_RAM DS1307::readBuffer()
{
	DS1307_RAM r;

	if (_use_hw)
	{
		// Set slave address and number of internal address bytes.
		twi->TWI_MMR = (1 << 8) | TWI_MMR_MREAD | (DS1307_ADDR << 16);
		// Set internal address bytes
		twi->TWI_IADR = 8;
		// Send START condition to start reading
		twi->TWI_CR = TWI_CR_START;
		for (int i=0; i<55; i++)
		{
			while ((twi->TWI_SR & TWI_SR_RXRDY) != TWI_SR_RXRDY) {};
			r.cell[i] = twi->TWI_RHR;
		}

		twi->TWI_CR = TWI_CR_STOP;
		while ((twi->TWI_SR & TWI_SR_RXRDY) != TWI_SR_RXRDY) {};
		r.cell[55] = twi->TWI_RHR;
		while ((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {};
	}
	else
	{
		_sendStart(DS1307_ADDR_W);
		_waitForAck();
		_writeByte(8);
		_waitForAck();
		_sendStop();
		_sendStart(DS1307_ADDR_R);
		_waitForAck();

		for (int i=0; i<56; i++)
		{
			r.cell[i] = _readByte();
			if (i<55)
				_sendAck();
			else
				_sendNack();
		}
		_sendStop();
	}

	return r;
}

void DS1307::poke(uint8_t addr, uint8_t value)
{
	if ((addr >=0) && (addr<=55))
	{
		addr += 8;
		if (_use_hw)
		{
			// Set slave address and number of internal address bytes.
			twi->TWI_MMR = (1 << 8) | (DS1307_ADDR << 16);
			// Set internal address bytes
			twi->TWI_IADR = addr;
			// Send a single byte to start transfer
			twi->TWI_THR = value;
			while ((twi->TWI_SR & TWI_SR_TXRDY) != TWI_SR_TXRDY) {};
			// Send STOP condition
			twi->TWI_CR = TWI_CR_STOP;
			while ((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {};
		}
		else
		{
			_sendStart(DS1307_ADDR_W);
			_waitForAck();
			_writeByte(addr);
			_waitForAck();
			_writeByte(value);
			_waitForAck();
			_sendStop();
		}
	}
}

uint8_t DS1307::peek(uint8_t addr)
{
	if ((addr >=0) && (addr<=55))
	{
		uint8_t readValue;

		addr += 8;
		if (_use_hw)
		{
			// Set slave address and number of internal address bytes.
			twi->TWI_MMR = (1 << 8) | TWI_MMR_MREAD | (DS1307_ADDR << 16);
			// Set internal address bytes
			twi->TWI_IADR = addr;
			// Send START and STOP condition to read a single byte
			twi->TWI_CR = TWI_CR_START | TWI_CR_STOP;
			while ((twi->TWI_SR & TWI_SR_RXRDY) != TWI_SR_RXRDY) {};
			readValue = twi->TWI_RHR;
			while ((twi->TWI_SR & TWI_SR_TXCOMP) != TWI_SR_TXCOMP) {};
		}
		else
		{
			_sendStart(DS1307_ADDR_W);
			_waitForAck();
			_writeByte(addr);
			_waitForAck();
			_sendStop();
			_sendStart(DS1307_ADDR_R);
			_waitForAck();
			readValue = _readByte();
			_sendNack();
			_sendStop();
		}

		return readValue;
	}
	else
		return 0;
}
