

void SetupPWM16() {
  DDRB |= _BV(PB1) | _BV(PB2);
  TCCR1A = _BV(COM1A1) | _BV(COM1B1) | _BV(WGM11);
  TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
  ICR1 = 0x7fff;  //15-bit resolution
}

void analogWrite16(uint8_t pin, uint16_t val){
  switch(pin){
    case  9: OCR1A = val; break;
    case 10: OCR1B = val; break;
  }
}

uint16_t analogRead16(uint8_t pin){
  switch(pin){
    case  9: return OCR1A;
    case 10: return OCR1B;
  }
}
