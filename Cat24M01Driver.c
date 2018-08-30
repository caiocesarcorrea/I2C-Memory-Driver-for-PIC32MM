/****************************************************************************************************************
 *                 Mem�ria CAT34M01 - Driver                                                                    *
 *                                                                                                              *
 ****************************************************************************************************************
 * Nome do Arquivo:     HardwareProfile.h                                                                       *
 * Dependencias:    	Veja a se��o INCLUDES abaixo                                                            *
 * Microcontrolador:                                                                                            *
 * Op��o de Clock:                                                                                              *
 * Compilador:        	XC32                                                                                    *
 * Empresa:         	SEMPRA                                                                                  *
 * Plataforma:		    AC8                                                                                     *
 * Autores:		        Bruno Avelar e Caio Corr�a                                                              *
 * Vers�o:		        v1.0                                                                                    *
 * Descri��o:                                                                                                   *
 ****************************************************************************************************************
 ****************************************************************************************************************
 * Log de modifica��es:                                                                                         *
 * Rev   Data         Descri��o                                                                                 *
 * 0.1   08/05/2018   Inicio do Projeto - Bruno S. Avelar                                                       *
 * 0.5   25/07/2018   Adi��o das fun��es de leitura e escrita sem interrup��o - Caio Corr�a                     *
 * 1.0   30/07/2018   Adi��o de fun��es auxiliares - Caio Corr�a                                                *
 *                                                                                                              *
 ***************************************************************************************************************/


#include "Cat24M01Driver.h"




/******************************************************************************
 * Funcao:      void i2c3Open(void)
 
 * Entrada:     Nenhuma (void)
 
 * Sa�da:       Nenhuma (void)
 
 * Descri��o:   Inicializa o m�dulo I2C3
 *****************************************************************************/

void i2c3Open(void){
        
    
        ANSELBbits.ANSB13 = 0 ;//Pino RB13 (SCL3) configurado como I/O digital
        
        
        
     //   I2C3BRG = 0x0b;//Seta o baudrate para 400kHz (  para SYSCLK = 24MHz )
        I2C3BRG = 0x73;//Seta o baudrate para 100kHz (  para SYSCLK = 24MHz )
     
    I2C3CONbits.ON = 0; //Disables the I2Cx module; all I2C pins are controlled by port functions
    
    I2C3CONbits.PCIE = 0;       // Stop detection interrupts are disabled
    I2C3CONbits.SCIE = 0;       // Start detection interrupts are disabled
    I2C3CONbits.BOEN = 0;       // I2CxRCV is only updated when the I2COV bit (I2CxSTAT<6>) is clear
    I2C3CONbits.SDAHT = 0;      // Minimum of 300 ns hold time on SDAx after the falling edge of SCLx
    I2C3CONbits.SBCDE = 0;      // Slave bus collision interrupts are disabled
    I2C3CONbits.SIDL = 1;       // Discontinues module operation when device enters Idle mode
    I2C3CONbits.SCLREL = 0;     // 1 = Releases SCLx clock
    I2C3CONbits.STRICT = 0;     // Strict I2C reserved address rule is not enabled
    I2C3CONbits.A10M = 0;       // I2CxADD is a 7-bit slave address
    I2C3CONbits.DISSLW = 0;     // Slew rate control is disabled
    I2C3CONbits.SMEN = 0;       // Disables SMBus input thresholds
    I2C3CONbits.GCEN = 0;       // General call address is disabled
    I2C3CONbits.STREN = 0;      // Disables software or receives clock stretching
    
    //Limpa registrador de status
    I2C3STAT = 0x0;
    
    I2C3CONbits.ON = 1; //Disables the I2Cx module; all I2C pins are controlled by port functions
    
    
    
}

/******************************************************************************
 * Funcao:      int WriteDataMemory(int addr, char data)
 
 * Entrada:     int addr - endere�o de mem�ria (17 bits, maior que zero)
                int data - dado a ser armazenado na mem�ria (16bits maior que zero)
 
 * Sa�da:       Nenhuma (void)
 
 * Retornos:    1 - Opera��o conclu�da com sucesso
                -1 - Erro: Falha no primeiro ACK
                -2 - Erro: Falha no segundo ACK
                -3 - Erro: Falha no terceiro ACK
                -4 - Erro: Falha no quarto ACK
                -10 - Erro: Colis�o no barramento
                -20 - Erro: Endere�o de mem�ria inv�lido
                -30 - Erro: Timeout

 * Descri��o:   Deposita o dado desejado no m�dulo de mem�ria CAT 24M01 atrav�s
 * do barramento I2C - Endereco m�ximo 131071 em 512 p�ginas de 256 bytes
 * M�ximo de 256 bytes por transa��o.
 *****************************************************************************/
int WriteDataMemory(int addr, char data){


//Separa o d�cimo s�timo bit
	char bitHigh = (char)addr >>16;

//Separa bytes mais e menos significativos do endere�o
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;
    int  timeOut = 0;
	
	

	if(bitHigh>1){
		return -20;
    //Erro 0: endere�o de mem�ria inv�lido
	}

	if(addr < 0){
		return -20;
    //Erro 0: endere�o de mem�ria inv�lido
	}


//1- Envia uma condi��o de start
	
       //Aguarda o barramento estar em idle
    while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    //Envia uma condi��o de start
	I2C3CONbits.SEN = 1;
       //Aguarda a condi��o de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    if(I2C3STATbits.BCL==1)
		return -10;
	
    timeOut=0;
	
//2-Envia o endere�o do slave (considerando o 16 bit)
        //Envio do dado
        //Inser��o do bit mais significativo no pacote de endere�o
	
	if(bitHigh == 0){
		I2C3TRN = MEMADDRL_W;   
	}else{
		I2C3TRN = MEMADDRH_W;
	}

    //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT ==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -1;
        //Erro 1: Primeiro ACK nao recebido pelo m�dulo de mem�ria
	}        

//3 - Envia primeiro byte de endere�o (bits 15 a 8)
    //Envio do dado

	I2C3TRN = addressHigh;
	
    //Aguarda o dado se propagar pelo barramento
	while(I2C3STATbits.TRSTAT ==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -2;
        //Erro 2: Segundo ACK nao recebido pelo m�dulo de mem�ria
	}        





//4 - Segundo byte de endere�o (bits 7 a 0)
	I2C3TRN = addressLow;
	
    //Aguarda o dado ir
 	while(I2C3STATbits.TRSTAT ==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{

		return -3;
        //Erro 3: Terceiro ACK nao recebido do m�dulo de mem�ria
	}  

    //Envia um byte de dado
	I2C3TRN = data;
        
    //Aguarda o dado ir
  while(I2C3STATbits.TRSTAT ==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		return -4;
        //Erro 4: Quarto ACK n�o recebido do m�dulo de mem�ria
	}  


    //STOP
    //Inicia um stop
	I2C3CONbits.PEN = 1; 
	while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return 1;
    }
    timeOut=0;
    
    return 1;

   //Sucesso

}//fim WriteDataMemory


/******************************************************************************
 * Funcao:      int WriteNDataMemory(int addr, char *data,int len);
 
 * Entrada:     int addr - endere�o de mem�ria (17 bits, maior que zero)
                char* data - dados a serem armazenados na  mem�ria (16bits maior que zero)
 *              int len - tamanho em bytes dos dados a serem armazenados 
 * 
 * Sa�da:        Nenhuma (void)
 
 * Retornos:    0~255 - Numero de bytes escritos, para operacao bem sucedida, 
 *                      o valor de retorno deve ser igual ao argumento de entrada int len
 *   
 *               -1 - Erro: Falha no primeiro ACK
 *               -2 - Erro: Falha no segundo ACK
 *               -3 - Erro: Falha no terceiro ACK
 *               -4 - Erro: Falha no quarto ACK
 *               -5 - Erro: Falha no quinto ACK
 *               -10 - Erro: Colis�o no barramento
 *               -20 - Erro: Endere�o de mem�ria inv�lido
 *               -30 - Erro: Timeout
 *               
 *
 * Descri��o:   Deposita o dado desejado no m�dulo de mem�ria CAT 24M01 atrav�s
 * do barramento I2C - Endereco m�ximo 131071 em 512 p�ginas de 256 bytes
 * M�ximo de 256 bytes por transa��o.
 *****************************************************************************/
int WriteNDataMemory(int addr, char *data, int len){

	
    //Iterador para o loop
	int i = 0;
    int timeOut = 0;
    //Separa o d�cimo s�timo bit
	char bitHigh = (char)addr >>16;

    //Separa bytes mais e menos significativos do endere�o
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;

	
	

	if(bitHigh>1){
		return -20;
        //Erro 20: endere�o de mem�ria inv�lido
	}

	if(addr < 0){
		return -20;
      //Erro 20: endere�o de mem�ria inv�lido
	}


//1- Envia uma condi��o de start
	
       //Aguarda o barramento estar em idle
  while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    //Envia uma condi��o de start
	I2C3CONbits.SEN = 1;
    
    //Aguarda a condi��o de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
     if(I2C3STATbits.BCL==1)
		return -10;
    timeOut=0;
	
//2-Envia o endere�o do slave (considerando o 16 bit)
        //Envio do dado
        //Inser��o do bit mais significativo no pacote de endere�o
	
	
	if(bitHigh == 0){
		I2C3TRN = MEMADDRL_W;   
	}else{
		I2C3TRN = MEMADDRH_W;
	}
        //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -1;
        //Erro 1: Primeiro ACK nao recebido pelo m�dulo de mem�ria
	}        

//3 - Envia primeiro byte de endere�o (bits 15 a 8)
    //Envio do dado

	I2C3TRN = addressHigh;;
	
    //Aguarda o dado se propagar pelo barramento
  	while(I2C3STATbits.TRSTAT ==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
  
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -2;
        //Erro 2: Segundo ACK nao recebido pelo m�dulo de mem�ria
	}        



	

//4 - Segundo byte de endere�o (bits 7 a 0)
	I2C3TRN = addressLow;
	
    //Aguarda o dado ir
  	while(I2C3STATbits.TRSTAT ==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
  
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{

		return -3;
        //Erro 3: Terceiro ACK nao recebido do m�dulo de mem�ria
	}  

    //Envia N bytes um byte de dado
	for(i=0; i<len; i++){
		
		I2C3TRN = *(data+i);
        //Aguarda o dado ir
		
		while(I2C3STATbits.TRSTAT){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
		
		
		
        //Verifica se recebeu um ack
		if(I2C3STATbits.ACKSTAT == 1)
		{
			return -4;
            //Erro 4: Quarto ACK n�o recebido do m�dulo de mem�ria
		}  

	}
    
    //STOP
    //Inicia um stop
	I2C3CONbits.PEN = 1; 
	while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return i;
    }
    
	
	return i;

   //Sucesso

}//fim WriteNDataMemory











/******************************************************************************
 * Funcao:      char ReadDataMemory(int addr, char *data)
 
 * Entrada:     int addr - endere�o de mem�ria (17 bits, maior que zero)
                
 * 
 * Sa�da:       char *data - Valor lido da mem�ria
 
 
 * Retornos:    0 - Erro: Endere�o de mem�ria inv�lido
 *              -1 - Erro: Falha no primeiro ACK
 *              -2 - Erro: Falha no segundo ACK
 *              -3 - Erro: Falha no terceiro ACK
 *              -4 - Erro: Falha no quarto ACK
 *              -10 - Erro: Colis�o no barramento
 *              -20 - Erro: Endere�o de mem�ria inv�lido
 *              -30 - Erro: Timeout   
 * 
 * 
 * Descri��o:   Le o byte desejado no m�dulo de mem�ria CAT 24M01 atrav�s
 * do barramento I2C - Endereco m�ximo 131071 em 512 p�ginas de 256 bytes
 *****************************************************************************/

int ReadDataMemory(int addr, char *data){

//Separa o d�cimo s�timo bit
	char bitHigh = (char)addr >>16;
	
//Separa bytes mais e menos significativos do endere�o
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;
    
    int timeOut = 0;
	
	

	if(bitHigh>1){
		return -20;
    //Erro 20: endere�o de mem�ria inv�lido
	}

	if(addr < 0){
		return -20;
    //Erro 20: endere�o de mem�ria inv�lido
	}

//Realiza uma opera��o de escrita em falso (dummy write) em um endere�o 
// imediatamente abaixo do endere�o a ser lido da mem�ria (immediate address read)

    //1- Envia uma condi��o de start
	
       //Aguarda o barramento estar em idle
  	while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
       //Envia uma condi��o de start
	I2C3CONbits.SEN = 1;
       //Aguarda a condi��o de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	if(I2C3STATbits.BCL==1)
		return -10;
	
//2-Envia o endere�o do slave (considerando o 16 bit)
        //Envio do dado
        //Inser��o do bit mais significativo no pacote de endere�o
	if(bitHigh == 0){
		I2C3TRN = MEMADDRL_W;   
	}else{
		I2C3TRN = MEMADDRH_W;
	}

    //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;

    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -1;
        //Erro 1: Primeiro ACK nao recebido pelo m�dulo de mem�ria
	}        

//3 - Envia primeiro byte de endere�o (bits 15 a 8)
    //Envio do dado

	I2C3TRN = addressHigh;
	
    //Aguarda o dado se propagar pelo barramento
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;

    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -2;
        //Erro 2: Segundo ACK nao recebido pelo m�dulo de mem�ria
	}        





//4 - Segundo byte de endere�o (bits 7 a 0)
	I2C3TRN = addressLow;
	
    //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;

        //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{

		return -3;
           //Erro 3: Terceiro ACK nao recebido do m�dulo de mem�ria
	}  

	
//Envia segunda condi��o de start para transacao de Immediate Address Read
     
    
        //Envia uma condi��o de restart
	I2C3CONbits.RSEN = 1;
       //Aguarda a condi��o de restart propagar pelo barramento
	while(I2C3CONbits.RSEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;



//Inser��o do bit mais significativo no pacote de endere�o
	if(bitHigh == 0){
		I2C3TRN = MEMADDRL_R;   
	}else{
		I2C3TRN = MEMADDRH_R;
	}



    //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
        
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -4;
        //Erro 4: Quarto ACK nao recebido pelo m�dulo de mem�ria
	}        
    
    //Habilita recebimento de dado
	I2C3CONbits.RCEN=1;
    //Prepara um NACK
	I2C3CONbits.ACKDT=1;
	while(I2C3CONbits.RCEN==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    
    
    //Envia o NACK
	I2C3CONbits.ACKEN =1;
	
    *data = I2C3RCV;
	
    //Aguarda final da transmiss�o do NACK
    while(I2C3CONbits.ACKEN == 1 ){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    //STOP
    //Inicia e aguarda condi��o de stop
    I2C3CONbits.PEN = 1; 
	while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    
	return 1;

   //Sucesso

}// fim ReadDataMemory




/******************************************************************************
 * Funcao:      char ReadNDataMemory(int addr, char *data,int len);
 
 * Entrada:     int address - endere�o de mem�ria (17 bits, maior que zero)
                int len - n�mero de bytes a serem lidos da mem�ria
 *  
 * Sa�da:       char *data - Valores lidos da mem�ria
 * Retornos:    1~255 - ByteLido da mem�ria com sucesso
                0 - Erro: Endere�o de mem�ria inv�lido
 *              -1 - Erro: Falha no primeiro ACK
 *              -2 - Erro: Falha no segundo ACK
 *              -3 - Erro: Falha no terceiro ACK
 *              -4 - Erro: Falha no quarto ACK
 *              -10 - Erro: Colis�o no barramento
 *              -20 - Erro: Endere�o de mem�ria inv�lido
 *              -30 - Erro: Timeout   
 * 
                
 * Descri��o:   Le os N bytes desejados no m�dulo de mem�ria CAT 24M01. O m�dulo
 * permite no m�ximo 255 opera��es por acesso atrav�s do barramento I2C
 * Endereco m�ximo 131071 em 512 p�ginas de 256 bytes
 *****************************************************************************/

int ReadNDataMemory(int addr, char *data, int len){

    //Iterador para o loop 
	int i=0;
//Separa o d�cimo s�timo bit
	char bitHigh = (char)addr >>16;
	
//Separa bytes mais e menos significativos do endere�o
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;
    int timeOut = 0;
	
	

	if(bitHigh>1){
		return 0;
    //Erro 0: endere�o de mem�ria inv�lido
	}

	if(addr < 0){
		return 0;
    //Erro 0: endere�o de mem�ria inv�lido
	}

//Realiza uma opera��o de escrita em falso (dummy write) em um endere�o 
// imediatamente abaixo do endere�o a ser lido da mem�ria (immediate address read)

    //1- Envia uma condi��o de start
	
       //Aguarda o barramento estar em idle
    while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    //Envia uma condi��o de start
	I2C3CONbits.SEN = 1;

    //Aguarda a condi��o de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
     if(I2C3STATbits.BCL==1)
		return -10;
    timeOut=0;
    
	if(I2C3STATbits.BCL==1)
		return -10;
	
//2-Envia o endere�o do slave (considerando o 16 bit)
        //Envio do dado
        //Inser��o do bit mais significativo no pacote de endere�o
	if(bitHigh == 0){
		I2C3TRN = MEMADDRL_W;   
	}else{
		I2C3TRN = MEMADDRH_W;
	}

    //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
  
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -1;
            //Erro 1: Primeiro ACK nao recebido pelo m�dulo de mem�ria
	}        

//3 - Envia primeiro byte de endere�o (bits 15 a 8)
    //Envio do dado

	I2C3TRN = addressHigh;
	
    //Aguarda o dado se propagar pelo barramento
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
        //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -2;
            //Erro 1: Segundo ACK nao recebido pelo m�dulo de mem�ria
	}        





//4 - Segundo byte de endere�o (bits 7 a 0)
	I2C3TRN = addressLow;
	
    //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    
        //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{

		return -3;
           //Erro 3: Terceiro ACK nao recebido do m�dulo de mem�ria
	}  

	
//Envia segunda condi��o de start para transacao de Immediate Address Read


        //Envia uma condi��o de restart
	I2C3CONbits.RSEN = 1;
       //Aguarda a condi��o de restart propagar pelo barramento
	while(I2C3CONbits.RSEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;



//Inser��o do bit mais significativo no pacote de endere�o
	if(bitHigh == 0){
		I2C3TRN = MEMADDRL_R;   
	}else{
		I2C3TRN = MEMADDRH_R;
	}



    //Aguarda o dado ir
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    //Verifica se recebeu um ack
	if(I2C3STATbits.ACKSTAT == 1)
	{
		
		return -4;
        //Erro 4: Primeiro ACK nao recebido pelo m�dulo de mem�ria
	}        
	
	for(i=0;i<len;i++){
    //Habilita recebimento de dado
		I2C3CONbits.RCEN=1;
    //Prepara um ACK
		I2C3CONbits.ACKDT=0;
    while(I2C3CONbits.RCEN==1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;

		
    //Envia o ACK ou NACK, se for o ultimo bit
	if(i==len-1) I2C3CONbits.ACKEN = 0;
    else    I2C3CONbits.ACKEN = 1;
        
    //Aguarda o ACK propagar no barramento    
    while(I2C3CONbits.ACKEN == 1 ){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    //Armazena byte lido no buffer	
    *(data+i) = I2C3RCV;
		
	}
    
    
    //STOP
    //Inicia condi��o de stop
	I2C3CONbits.PEN = 1; 
	while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }

	
	return i;

   //Sucesso

}// fim ReadNDataMemory




/******************************************************************************
 * Funcao:      int isMemoryReady(void);
 
 * Entrada:     Nenhuma
 
 * Sa�da:       Nenhuma
 
 * Retornos:    1 - M�dulo de mem�ria livre
                0 - Erro: M�dulo de mem�ria ocupado
                -10 - Erro: Colis�o no barramento
                -30 - Erro: Timeout
 * Descri��o:   Envia uma escriva vazia (dummy write) e caso receba um NACK,
 * o m�dulo de mem�ria est� ocupado escrevendo dados
 *****************************************************************************/



int isMemoryReady(){
    
    int timeOut = 0 ;
	//Aguarda barramento estar em idle
    while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
      //Envia uma condi��o de start
	I2C3CONbits.SEN = 1;
       //Aguarda a condi��o de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	if(I2C3STATbits.BCL==1)
		return -10;
	
//2-Envia o endere�o do slave (considerando o 16 bit)
        //Envio do dado
        //Inser��o do bit mais significativo no pacote de endere�o
	
	I2C3TRN = MEMADDRH_W;
	
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    //Testa se recebeu um NACK - M�dulo ocupado
    if(I2C3STATbits.ACKSTAT == 1)
	{
        //Inicia e aguarda condi��o de STOP se propagar
		I2C3CONbits.PEN = 1; 
		while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
		return 0;
        //Erro 0: M�dulo de mem�ria ocupado
	}
    
	if(I2C3STATbits.IWCOL){
	
        //Inicia e aguarda condi��o de STOP se propagar
        I2C3CONbits.PEN = 1; 
		while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
		return -1;
        //Erro 1: Colis�o no barramento

	}
    
    //Inicia e aguarda condi��o de STOP se propagar
	I2C3CONbits.PEN = 1; 
	while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    return 1;
	
} // fim isMemoryReady()