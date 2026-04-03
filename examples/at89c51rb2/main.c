#include <8052.h>

void delay(int time)
{ 
    for (int i = 0; i < time; i++)
    {
        for (int j = 0; j < 1275; j++)
        {
            //
        }
    }
}
void main()
{    
    while(1)
    { 
        P0=0x00; 
        delay(100); 

        P0=0xFF; 
        delay(100); 
    }
}

