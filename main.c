#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

uint8_t LireBoutton()
{
    return PINB & (1<<PB3);
}

uint8_t LireFeu()
{
    return PINB & ((1<<PB2)|(1<<PB1)|(1<<PB0));
}

void EcrireFeu( uint8_t feu )
{
    PORTB = (PINB & 0xF8) | feu | (1<<PB3); //Il faut conserver les pull ups.
}

void Timer0_Init()
{
        TIMSK |= (0<<TOIE0)|(0<<OCIE0B)|(1<<OCIE0A);
	TCCR0A = (0<<COM0A1)|(0<<COM0A0)|(1<<WGM01)|(0<<WGM00);
	TCCR0B = (0<<WGM02)|(0<<CS02)|(1<<CS01)|(0<<CS00);
	TCNT0  = 0;
        OCR0A  = 160; // environ 10 ms
}

void Timer1_Init()
{
        TIMSK |= (1<<TOIE1);
	TCCR1 = (0<<CTC1)|(0<<COM1A1)|(0<<COM1A0)|(1<<CS13)|(1<<CS12)|(1<<CS11)|(1<<CS10);
        GTCCR = 0;
	TCNT1 = 0;
}

void ChangerLumiere()
{
        //Lire port B 0-2
        uint8_t feu0 = LireFeu();
        feu0 = feu0 << 1;

        if( feu0==0x08 )
        {
            feu0 = 0x01;
        }

        EcrireFeu( feu0 );
}

enum EtatBoutton { Relache=0x00, Presse=0x01, PresseSansRebond=0x02 };
volatile uint8_t _boutton = 0x00;

void Change()
{
    volatile uint8_t toto = 0x00;
    if( toto==0x00)
    {
        PORTB ^= (1<<PB4); //Il faut conserver les pull ups.
        toto = 0x01;
    }
    else
    {
        PORTB ^= (1<<PB4); //Il faut conserver les pull ups.
        toto = 0x00;
    }
}

void PortB_Init()
{
        DDRB  = 0x07; //mettre les 3 premiers bits du port B en sortie
        PORTB = (1<<PB5)|(1<<PB4)|(1<<PB3);  //Pull-up sur le button et sur reset
}

volatile uint8_t tempsInutilise = 0x00;
volatile uint8_t feuDurantDodo  = 0x01;

ISR(PCINT0_vect)
{
   cli();
   PCMSK &= ~(1<<PCINT3); //Activer PCInt pour le reveil
   GIMSK &= ~(1<<PCIE);
   sleep_disable();
   EcrireFeu( feuDurantDodo );
   TCNT1 = 0;
   tempsInutilise = 0;
   _boutton = PresseSansRebond;
   sei();
}

ISR(TIMER1_OVF_vect)
{
   cli();
   tempsInutilise ++;
   if( tempsInutilise==0x04)
   {
       feuDurantDodo = LireFeu();
       PORTB &= ~((1<<PB2)|(1<<PB1)|(1<<PB0)); //Eteindre toutes les lumieres
       PCMSK |= (1<<PCINT3); //Activer PCInt pour le reveil
       GIMSK |= (1<<PCIE);
       set_sleep_mode(SLEEP_MODE_PWR_DOWN);
       sleep_enable();
       sei();
       sleep_cpu();
   }
   else
   {
      sei();
   }
}

ISR(TIMER0_COMPA_vect)
{
    cli();
    uint8_t int0 = LireBoutton();
    if( int0==Relache ) //Presse
    {
        tempsInutilise = 0; //Recommence a 0
        TCNT1 = 0;
        if( _boutton==Relache )
        {
            _boutton = Presse;
        }
        else if( _boutton==Presse )
        {
            _boutton = PresseSansRebond;
            ChangerLumiere();
        }
    }
    else 
    {
        _boutton = Relache;
    }

    sei();
}

int main (void)
{
        GIMSK = 0;
        PCMSK = 0;
        PortB_Init();

	//Initialisation du compteur pour le delai
	Timer0_Init();
	Timer1_Init();

	//Valeur initiale
        EcrireFeu( 1 );

	sei(); //Activation des interruptions

	while(1)
	{
        }
}

