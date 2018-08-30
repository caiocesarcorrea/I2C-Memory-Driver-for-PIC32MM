/****************************************************************************************************************
 *                 Memória CAT34M01 - Driver                                                                    *
 *                                                                                                              *
 ****************************************************************************************************************
 * Nome do Arquivo:     HardwareProfile.h                                                                       *
 * Dependencias:    	Veja a seção INCLUDES abaixo                                                            *
 * Microcontrolador:                                                                                            *
 * Opção de Clock:                                                                                              *
 * Compilador:        	XC32                                                                                    *
 * Empresa:         	SEMPRA                                                                                  *
 * Plataforma:		    AC8                                                                                     *
 * Autores:		        Bruno Avelar e Caio Corrêa                                                              *
 * Versão:		        v1.0                                                                                    *
 * Descrição:                                                                                                   *
 ****************************************************************************************************************
 ****************************************************************************************************************
 * Log de modificações:                                                                                         *
 * Rev   Data         Descrição                                                                                 *
 * 0.1   08/05/2018   Inicio do Projeto - Bruno S. Avelar                                                       *
 * 0.5   25/07/2018   Adição das funções de leitura e escrita sem interrupção - Caio Corrêa                     *
 * 1.0   30/07/2018   Adição de funções auxiliares - Caio Corrêa                                                *
 *                                                                                                              *
 ***************************************************************************************************************/


#include "Cat24M01Driver.h"




/******************************************************************************
 * Funcao:      void i2c3Open(void)
 
 * Entrada:     Nenhuma (void)
 
 * Saída:       Nenhuma (void)
 
 * Descrição:   Inicializa o módulo I2C3
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
 
 * Entrada:     int addr - endereço de memória (17 bits, maior que zero)
                int data - dado a ser armazenado na memória (16bits maior que zero)
 
 * Saída:       Nenhuma (void)
 
 * Retornos:    1 - Operação concluída com sucesso
                -1 - Erro: Falha no primeiro ACK
                -2 - Erro: Falha no segundo ACK
                -3 - Erro: Falha no terceiro ACK
                -4 - Erro: Falha no quarto ACK
                -10 - Erro: Colisão no barramento
                -20 - Erro: Endereço de memória inválido
                -30 - Erro: Timeout

 * Descrição:   Deposita o dado desejado no módulo de memória CAT 24M01 através
 * do barramento I2C - Endereco máximo 131071 em 512 páginas de 256 bytes
 * Máximo de 256 bytes por transação.
 *****************************************************************************/
int WriteDataMemory(int addr, char data){


//Separa o décimo sétimo bit
	char bitHigh = (char)addr >>16;

//Separa bytes mais e menos significativos do endereço
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;
    int  timeOut = 0;
	
	

	if(bitHigh>1){
		return -20;
    //Erro 0: endereço de memória inválido
	}

	if(addr < 0){
		return -20;
    //Erro 0: endereço de memória inválido
	}


//1- Envia uma condição de start
	
       //Aguarda o barramento estar em idle
    while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    //Envia uma condição de start
	I2C3CONbits.SEN = 1;
       //Aguarda a condição de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    if(I2C3STATbits.BCL==1)
		return -10;
	
    timeOut=0;
	
//2-Envia o endereço do slave (considerando o 16 bit)
        //Envio do dado
        //Inserção do bit mais significativo no pacote de endereço
	
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
        //Erro 1: Primeiro ACK nao recebido pelo módulo de memória
	}        

//3 - Envia primeiro byte de endereço (bits 15 a 8)
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
        //Erro 2: Segundo ACK nao recebido pelo módulo de memória
	}        





//4 - Segundo byte de endereço (bits 7 a 0)
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
        //Erro 3: Terceiro ACK nao recebido do módulo de memória
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
        //Erro 4: Quarto ACK não recebido do módulo de memória
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
 
 * Entrada:     int addr - endereço de memória (17 bits, maior que zero)
                char* data - dados a serem armazenados na  memória (16bits maior que zero)
 *              int len - tamanho em bytes dos dados a serem armazenados 
 * 
 * Saída:        Nenhuma (void)
 
 * Retornos:    0~255 - Numero de bytes escritos, para operacao bem sucedida, 
 *                      o valor de retorno deve ser igual ao argumento de entrada int len
 *   
 *               -1 - Erro: Falha no primeiro ACK
 *               -2 - Erro: Falha no segundo ACK
 *               -3 - Erro: Falha no terceiro ACK
 *               -4 - Erro: Falha no quarto ACK
 *               -5 - Erro: Falha no quinto ACK
 *               -10 - Erro: Colisão no barramento
 *               -20 - Erro: Endereço de memória inválido
 *               -30 - Erro: Timeout
 *               
 *
 * Descrição:   Deposita o dado desejado no módulo de memória CAT 24M01 através
 * do barramento I2C - Endereco máximo 131071 em 512 páginas de 256 bytes
 * Máximo de 256 bytes por transação.
 *****************************************************************************/
int WriteNDataMemory(int addr, char *data, int len){

	
    //Iterador para o loop
	int i = 0;
    int timeOut = 0;
    //Separa o décimo sétimo bit
	char bitHigh = (char)addr >>16;

    //Separa bytes mais e menos significativos do endereço
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;

	
	

	if(bitHigh>1){
		return -20;
        //Erro 20: endereço de memória inválido
	}

	if(addr < 0){
		return -20;
      //Erro 20: endereço de memória inválido
	}


//1- Envia uma condição de start
	
       //Aguarda o barramento estar em idle
  while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    //Envia uma condição de start
	I2C3CONbits.SEN = 1;
    
    //Aguarda a condição de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
     if(I2C3STATbits.BCL==1)
		return -10;
    timeOut=0;
	
//2-Envia o endereço do slave (considerando o 16 bit)
        //Envio do dado
        //Inserção do bit mais significativo no pacote de endereço
	
	
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
        //Erro 1: Primeiro ACK nao recebido pelo módulo de memória
	}        

//3 - Envia primeiro byte de endereço (bits 15 a 8)
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
        //Erro 2: Segundo ACK nao recebido pelo módulo de memória
	}        



	

//4 - Segundo byte de endereço (bits 7 a 0)
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
        //Erro 3: Terceiro ACK nao recebido do módulo de memória
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
            //Erro 4: Quarto ACK não recebido do módulo de memória
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
 
 * Entrada:     int addr - endereço de memória (17 bits, maior que zero)
                
 * 
 * Saída:       char *data - Valor lido da memória
 
 
 * Retornos:    0 - Erro: Endereço de memória inválido
 *              -1 - Erro: Falha no primeiro ACK
 *              -2 - Erro: Falha no segundo ACK
 *              -3 - Erro: Falha no terceiro ACK
 *              -4 - Erro: Falha no quarto ACK
 *              -10 - Erro: Colisão no barramento
 *              -20 - Erro: Endereço de memória inválido
 *              -30 - Erro: Timeout   
 * 
 * 
 * Descrição:   Le o byte desejado no módulo de memória CAT 24M01 através
 * do barramento I2C - Endereco máximo 131071 em 512 páginas de 256 bytes
 *****************************************************************************/

int ReadDataMemory(int addr, char *data){

//Separa o décimo sétimo bit
	char bitHigh = (char)addr >>16;
	
//Separa bytes mais e menos significativos do endereço
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;
    
    int timeOut = 0;
	
	

	if(bitHigh>1){
		return -20;
    //Erro 20: endereço de memória inválido
	}

	if(addr < 0){
		return -20;
    //Erro 20: endereço de memória inválido
	}

//Realiza uma operação de escrita em falso (dummy write) em um endereço 
// imediatamente abaixo do endereço a ser lido da memória (immediate address read)

    //1- Envia uma condição de start
	
       //Aguarda o barramento estar em idle
  	while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
       //Envia uma condição de start
	I2C3CONbits.SEN = 1;
       //Aguarda a condição de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	if(I2C3STATbits.BCL==1)
		return -10;
	
//2-Envia o endereço do slave (considerando o 16 bit)
        //Envio do dado
        //Inserção do bit mais significativo no pacote de endereço
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
        //Erro 1: Primeiro ACK nao recebido pelo módulo de memória
	}        

//3 - Envia primeiro byte de endereço (bits 15 a 8)
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
        //Erro 2: Segundo ACK nao recebido pelo módulo de memória
	}        





//4 - Segundo byte de endereço (bits 7 a 0)
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
           //Erro 3: Terceiro ACK nao recebido do módulo de memória
	}  

	
//Envia segunda condição de start para transacao de Immediate Address Read
     
    
        //Envia uma condição de restart
	I2C3CONbits.RSEN = 1;
       //Aguarda a condição de restart propagar pelo barramento
	while(I2C3CONbits.RSEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;



//Inserção do bit mais significativo no pacote de endereço
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
        //Erro 4: Quarto ACK nao recebido pelo módulo de memória
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
	
    //Aguarda final da transmissão do NACK
    while(I2C3CONbits.ACKEN == 1 ){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    //STOP
    //Inicia e aguarda condição de stop
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
 
 * Entrada:     int address - endereço de memória (17 bits, maior que zero)
                int len - número de bytes a serem lidos da memória
 *  
 * Saída:       char *data - Valores lidos da memória
 * Retornos:    1~255 - ByteLido da memória com sucesso
                0 - Erro: Endereço de memória inválido
 *              -1 - Erro: Falha no primeiro ACK
 *              -2 - Erro: Falha no segundo ACK
 *              -3 - Erro: Falha no terceiro ACK
 *              -4 - Erro: Falha no quarto ACK
 *              -10 - Erro: Colisão no barramento
 *              -20 - Erro: Endereço de memória inválido
 *              -30 - Erro: Timeout   
 * 
                
 * Descrição:   Le os N bytes desejados no módulo de memória CAT 24M01. O módulo
 * permite no máximo 255 operações por acesso através do barramento I2C
 * Endereco máximo 131071 em 512 páginas de 256 bytes
 *****************************************************************************/

int ReadNDataMemory(int addr, char *data, int len){

    //Iterador para o loop 
	int i=0;
//Separa o décimo sétimo bit
	char bitHigh = (char)addr >>16;
	
//Separa bytes mais e menos significativos do endereço
	char addressHigh =(char) (addr>>8)%256;
	char addressLow = (char) addr%256;
    int timeOut = 0;
	
	

	if(bitHigh>1){
		return 0;
    //Erro 0: endereço de memória inválido
	}

	if(addr < 0){
		return 0;
    //Erro 0: endereço de memória inválido
	}

//Realiza uma operação de escrita em falso (dummy write) em um endereço 
// imediatamente abaixo do endereço a ser lido da memória (immediate address read)

    //1- Envia uma condição de start
	
       //Aguarda o barramento estar em idle
    while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
    //Envia uma condição de start
	I2C3CONbits.SEN = 1;

    //Aguarda a condição de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
     if(I2C3STATbits.BCL==1)
		return -10;
    timeOut=0;
    
	if(I2C3STATbits.BCL==1)
		return -10;
	
//2-Envia o endereço do slave (considerando o 16 bit)
        //Envio do dado
        //Inserção do bit mais significativo no pacote de endereço
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
            //Erro 1: Primeiro ACK nao recebido pelo módulo de memória
	}        

//3 - Envia primeiro byte de endereço (bits 15 a 8)
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
            //Erro 1: Segundo ACK nao recebido pelo módulo de memória
	}        





//4 - Segundo byte de endereço (bits 7 a 0)
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
           //Erro 3: Terceiro ACK nao recebido do módulo de memória
	}  

	
//Envia segunda condição de start para transacao de Immediate Address Read


        //Envia uma condição de restart
	I2C3CONbits.RSEN = 1;
       //Aguarda a condição de restart propagar pelo barramento
	while(I2C3CONbits.RSEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;



//Inserção do bit mais significativo no pacote de endereço
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
        //Erro 4: Primeiro ACK nao recebido pelo módulo de memória
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
    //Inicia condição de stop
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
 
 * Saída:       Nenhuma
 
 * Retornos:    1 - Módulo de memória livre
                0 - Erro: Módulo de memória ocupado
                -10 - Erro: Colisão no barramento
                -30 - Erro: Timeout
 * Descrição:   Envia uma escriva vazia (dummy write) e caso receba um NACK,
 * o módulo de memória está ocupado escrevendo dados
 *****************************************************************************/



int isMemoryReady(){
    
    int timeOut = 0 ;
	//Aguarda barramento estar em idle
    while((I2C3CON & 0x1F) != 0 && (I2C3STATbits.TRSTAT != 0)){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
      //Envia uma condição de start
	I2C3CONbits.SEN = 1;
       //Aguarda a condição de start propagar pelo barramento
	while(I2C3CONbits.SEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	if(I2C3STATbits.BCL==1)
		return -10;
	
//2-Envia o endereço do slave (considerando o 16 bit)
        //Envio do dado
        //Inserção do bit mais significativo no pacote de endereço
	
	I2C3TRN = MEMADDRH_W;
	
	while(I2C3STATbits.TRSTAT == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    //Testa se recebeu um NACK - Módulo ocupado
    if(I2C3STATbits.ACKSTAT == 1)
	{
        //Inicia e aguarda condição de STOP se propagar
		I2C3CONbits.PEN = 1; 
		while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
		return 0;
        //Erro 0: Módulo de memória ocupado
	}
    
	if(I2C3STATbits.IWCOL){
	
        //Inicia e aguarda condição de STOP se propagar
        I2C3CONbits.PEN = 1; 
		while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
		return -1;
        //Erro 1: Colisão no barramento

	}
    
    //Inicia e aguarda condição de STOP se propagar
	I2C3CONbits.PEN = 1; 
	while(I2C3CONbits.PEN == 1){
        if(timeOut++ > TIMEOUT_VALUE)return -30;
    }
    timeOut=0;
	
    return 1;
	
} // fim isMemoryReady()