/* PRACTICA 3:  Gina Catalina Baquero Mojica
 *              Juan Sebastian Barreto JImenez
 *              Juan Pablo Clavijo Caceres
 * Entrega:     Mayo 14 2020
 */

/*Prototype Functions*/ 
void setup_function(); // configuraciones basicas del sistema

/* Mascaras de Bits*/
#define RECEIVE_COMPLETE 0x80 // Usart
#define TRANSMIT_COMPLETE 0x40 // Usart
#define BUFFER_EMPTY 0x20 // Usart
#define RX_TX_ENABLE 0x18 // Usart
#define START_CONDITION 0xA4 // I2C
#define READ 0xA1 // Control Byte Read I2C
#define WRITE 0xA0 // Control Byte Write I2C

/* MACROS USART*/
#define buffer_empty()((UCSR0A & BUFFER_EMPTY) != 0)
#define rx_tx_enable()(UCSR0B = RX_TX_ENABLE)
#define rx_flag() ((UCSR0A & RECEIVE_COMPLETE) != 0)
#define rx_flag_down() (UCSR0A |= RECEIVE_COMPLETE)
#define tx_flag() ((UCSR0A & TRANSMIT_COMPLETE) != 0)
#define tx_flag_down() (UCSR0A |= TRANSMIT_COMPLETE)
#define write_buffer_usart(VALUE)(UDR0 = VALUE)
#define read_buffer_usart()(UDR0)
/* MACROS I2C*/
#define read_buffer_twi()(TWDR)
#define readFinish()((TWCR & 0x80) != 0)
#define read_status_code(VALUE) ((TWSR & 0xF8) == VALUE)
#define write_buffer_twi(VALUE)(TWDR = VALUE)
#define write_control_register(VALUE)(TWCR = VALUE)

/* Enumerates States FSM*/
/* FSM Write_1000 = W1, Read_1000 = R1, Read_data = RD, states = 29*/
typedef enum{
read_function_usart, write_1000, w1_receive_data, w1_setup_slaver_write, w1_send_address, w1_send_data, w1_receive_alert, w1_stop_slaver,
finish_write_1000, read_1000, r1_setup_slaver_write, r1_send_address, r1_setup_slaver_read, r1_recieve_data_memory,  r1_stop_slaver, r1_send_data,  
r1_receive_alert, finish_read_1000, read_data, rd_receive_number_data, rd_setup_slaver_write, rd_send_address, rd_setup_slaver_read, rd_recieve_data_memory,
rd_stop_slaver, rd_send_data, finish_read_data, error_I2C}STATE_T;

int main(void){
    setup_function();
    rx_tx_enable();
    STATE_T state = read_function_usart; // Se inicializa el primer estado
    int end_fsm_write_1000 = 0, end_fsm_read_1000 = 0, end_fsm_read_data = 0; // Se inicializan en 0 las banderas que indican el final de un proceso
    uint16_t contador_2000 = 0; // se incializa en 0 el contador de 2000
    uint8_t contador_2000_MSB = 0, contador_2000_LSB = 0, lectura; // se incializa en 0 el contador de 2000 MSB y LSB y la variable que recibe de usart la función
    int i = 0; // se inicializa en 0 el contador i
    uint8_t data_lsb, data_msb, number_data_msb, number_data_lsb; // se inicializan en 0 las variables que reciben de usart
    char alert;  // se inicializan en 0 las variable alert que reciben de usart
    while(1){
        switch (state) {
      /*------------------------- Function Usart --------------------------*/
            case read_function_usart:
                if(rx_flag()){
                    lectura = read_buffer_usart();
                    rx_flag_down();
                    if (lectura == 'W' && buffer_empty()) {
                        write_buffer_usart(0xAA); // ready_w1
                        state = write_1000;
                    } else if (lectura == 'R' && buffer_empty()) {
                        write_buffer_usart(0xBA); // ready_r1
                        state = read_1000;
                    } else if (lectura == 'D' && buffer_empty()) {
                        write_buffer_usart(0xCA); // ready_rd
                        state = read_data;
                    }
                }else{
                    state = read_function_usart;
                }
                break;
      /*-------------------------- Write 1000 ----------------------------*/
            case write_1000:  
                if (tx_flag()) {
                    tx_flag_down();
                    state = w1_receive_data;
                } else if (end_fsm_write_1000 == 1) {
                    end_fsm_write_1000 = 0;
                    state = read_function_usart;
                } else{
                    state = write_1000;
                }
                break;
        
            case w1_receive_data:
                if (rx_flag() && (i == 0)) {
                    rx_flag_down();
                    data_lsb = read_buffer_usart(); // Se recibe por USART el Byte menos significativo del dato
                    i++;
                    state = w1_receive_data;
                }else if(rx_flag() && (i == 1)){
                    rx_flag_down();
                    i = 0;
                    data_msb = read_buffer_usart(); // Se recibe por USART el Byte más significativo del dato
                    write_control_register(START_CONDITION); // Start Condition I2C
                    state = w1_setup_slaver_write;
                } else {
                    state = w1_receive_data;
                }
                break;
        
            case w1_setup_slaver_write:
                if(readFinish()){// Se lee que el TWINT este en '1'
                  if(read_status_code(0x08)){ // Se lee que la Start Condition halla sido recibida
                    write_buffer_twi(WRITE); // Control Byte de escritura
                    write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                    state = w1_send_address;
                  }else{
                    write_buffer_usart(0x01); // COD = 1
                    state = error_I2C;
                  }
                }
                break;
        
            case w1_send_address:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                  contador_2000_LSB = contador_2000; // Se toma el byte menos significativo del contador de 2000
                  contador_2000_MSB = (contador_2000>>8); // Se toma el byte más significativo del contador de 2000
                  if(read_status_code(0x18)){// El control byte y el ACK de escritura ha sido recibido   
                      write_buffer_twi(contador_2000_MSB); // Se manda el byte más significativo de la dirección de escritura
                      write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                      state = w1_send_address;
                  }else if(read_status_code(0x28)){// El dato ha sido transmitido y el ACK ha sido recibido
                      write_buffer_twi(contador_2000_LSB); // Se manda el byte menos significativo de la dirección de escritura
                      write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                      state = w1_send_data;
                  }else{
                      write_buffer_usart(0x19); // COD = 25
                      state = error_I2C;
                  }
                }
                break;
        
        
            case w1_send_data:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if(read_status_code(0x28) && (i == 0)){// El dato ha sido transmitido y el ACK ha sido recibido
                        write_buffer_twi(data_lsb);
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        i++;
                        state = w1_send_data;
                    }else if(read_status_code(0x28) && (i == 1)){// El dato ha sido transmitido y el ACK ha sido recibido
                        write_buffer_twi(data_msb);
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        i++;
                        state = w1_send_data;
                    }else if(read_status_code(0x28) && (i == 2)){// El dato ha sido transmitido y el ACK ha sido recibido
                        i = 0;
                        write_control_register(0x94); //STOP CONDITION 
                        state = w1_receive_alert;
                    }else{
                        write_buffer_usart(0x1B); // COD = 27
                        state = error_I2C;
                    }
                }
                break;
       
            case w1_receive_alert:
                if(rx_flag()){
                    rx_flag_down();
                    alert = read_buffer_usart();
                    state = w1_stop_slaver;
                }else {
                    state = w1_receive_alert;
                }
                break;
        
            case w1_stop_slaver:
                    if (contador_2000 < 2000 && alert == 'C') { // Continue
                        contador_2000 += 2;
                        state = w1_receive_data;
                    } else if (contador_2000 >= 2000 || alert == 'S') { // Stop
                        contador_2000 = 0;
                        write_buffer_usart(0xAB); // finish_w1
                        state = finish_write_1000;
                    } else {
                        state = w1_stop_slaver;
                    }
                break;
        
            case finish_write_1000:
                if(tx_flag()){
                    tx_flag_down();
                    end_fsm_write_1000 = 1;
                    state = write_1000;
                }else{
                    finish_write_1000;
                }
                break;
            /*-------------------------- Read 1000 ----------------------------*/
            case read_1000:
                if (tx_flag()) {
                    tx_flag_down();
                    write_control_register(START_CONDITION); // Start Condition I2C
                    state = r1_setup_slaver_write;
                } else if (end_fsm_read_1000 == 1) {
                    end_fsm_read_1000 = 0; 
                    state = read_function_usart;
                } else {
                    state = read_1000  ;
                } 
            break;
        
            case r1_setup_slaver_write:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                  if(read_status_code(0x08)){ // Se lee que la Start Condition halla sido recibida
                      write_buffer_twi(WRITE); // Control Byte de escritura
                      write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                      state = r1_send_address;
                  }else{
                      write_buffer_usart(0x30); // COD = 48
                      state = error_I2C;
                  }
                }
                break;
        
            case r1_send_address:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    contador_2000_LSB = contador_2000; // Se toma el byte menos significativo del contador de 2000
                    contador_2000_MSB = (contador_2000>>8); // Se toma el byte más significativo del contador de 2000
                    if(read_status_code(0x18)){// El control byte y el ACK de escritura ha sido recibido   
                        write_buffer_twi(contador_2000_MSB); // Se manda el byte más significativo de la dirección de escritura
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = r1_send_address;
                    }else if(read_status_code(0x28) && (i == 0)){// El dato ha sido transmitido y el ACK ha sido recibido
                        i++;
                        write_buffer_twi(contador_2000_LSB); // Se manda el byte menos significativo de la dirección de escritura
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = r1_send_address;
                    }else if(read_status_code(0x28) && (i == 1)){// El dato ha sido transmitido y el ACK ha sido recibido
                        i = 0; 
                        write_control_register(START_CONDITION); // Repeat Start Condition I2C
                        state = r1_setup_slaver_read;
                    }else{
                        write_buffer_usart(0x46); // COD = 70
                        state = error_I2C;
                    }
                }
                break;
        
            case r1_setup_slaver_read:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if(read_status_code(0x10)){// Se lee la repetcion del start condition
                        write_buffer_twi(READ); // Control Byte Read
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = r1_setup_slaver_read;
                    }else if(read_status_code(0x40)){ // El control byte y el ACK de lectura ha sido recibido
                        write_control_register(0xC4); // Se baja la bandera TWINT, se envia ACK y se mantiene en '1' TWEN
                        state = r1_recieve_data_memory;
                    }else{
                        write_buffer_usart(0x52); // COD = 82
                        state = error_I2C;
                    }
                }
                break;
        
            case r1_recieve_data_memory:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if(read_status_code(0x50)){ // El dato y el ACK se ha recibido
                        data_lsb = read_buffer_twi(); // Se recibe el byte menos significativo del dato a leer
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = r1_stop_slaver;
                    }else{
                        write_buffer_usart(0x54); // COD = 84
                        state = error_I2C;
                    }
                }
                break;
        
            case r1_stop_slaver:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if(read_status_code(0x58)){ // El dato y el NACK se ha recibido
                        data_msb = read_buffer_twi(); // Se recibe el byte más significativo del dato a leer
                        write_control_register(0x94); // STOP CONDITION
                        write_buffer_usart(data_lsb); // Se transmite por USART el dato menos significativo del dato leido
                        state = r1_send_data;
                    }else{
                        write_buffer_usart(0x57); // COD = 87
                        state = error_I2C;
                    }
                }
                break;
        
            case r1_send_data:
                if (tx_flag() && (i == 0)) {
                    tx_flag_down();
                    write_buffer_usart(data_msb); // Se transmite por USART el dato más significativo del dato leido
                    i++;
                    state = r1_send_data;
                } else if (tx_flag() && (i == 1)) {
                    tx_flag_down();
                    i = 0;
                    state = r1_receive_alert;
                }
                break;
        
            case r1_receive_alert:
                if (rx_flag()) {
                    rx_flag_down();
                    alert = read_buffer_usart();
                    if (contador_2000 < 2000 && alert == 'C') {
                        contador_2000 += 2;
                        write_control_register(START_CONDITION); // Start Condition
                        state = r1_setup_slaver_write;
                    } else if (contador_2000 >= 2000 || alert == 'S') {
                        contador_2000 = 0;
                        write_buffer_usart(0xBB); // finish_r1
                        state = finish_read_1000;
                    }
                } else {
                   state = r1_receive_alert;
                }
                break;
        
            case finish_read_1000:
                if(tx_flag()){
                    tx_flag_down();
                    end_fsm_read_1000 = 1;
                    state = read_1000;
                }else{
                    finish_read_1000;
                }
                break;
                
      /*-------------------------- Read Data ----------------------------*/
            case read_data:
                if (tx_flag()) {
                    tx_flag_down();
                    state = rd_receive_number_data;
                }else if (end_fsm_read_data == 1) {
                    end_fsm_read_data = 0;
                    state = read_function_usart;
                }else{
                    state = read_data;
                } 
                break;
      
                
            case rd_receive_number_data:
                if (rx_flag() && (i==0)){
                    rx_flag_down();
                    number_data_lsb = read_buffer_usart(); // Se recibe el byte menos significativo de la dirección del dato a leer
                    i++;
                    state = rd_receive_number_data;
                } else if (rx_flag () && (i==1)) {
                    rx_flag_down();
                    i = 0;
                    number_data_msb = read_buffer_usart(); // Se recibe el byte más significativo de la dirección del dato a leer
                    write_control_register(START_CONDITION);
                    state = rd_setup_slaver_write;
                } else {
                    state = rd_receive_number_data;
                }
                break;
                     
            case rd_setup_slaver_write:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if(read_status_code(0x08)){ // Se lee que la Start Condition halla sido recibida
                        write_buffer_twi(WRITE); // Control Byte de escritura
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = rd_send_address;
                    } else {
                        write_buffer_usart(0x65); // COD = 101
                        state = error_I2C;
                    }
                }
                break;
      
            case rd_send_address:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if (read_status_code(0x18)) {// El control byte y el ACK de escritura ha sido recibido   
                        write_buffer_twi(number_data_msb); // Se manda el byte más significativo de la dirección de escritura
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = rd_send_address;
                    } else if (read_status_code(0x28) && (i == 0)) {// El dato ha sido transmitido y el ACK ha sido recibido
                        i++;
                        write_buffer_twi(number_data_lsb); // Se manda el byte menos significativo de la dirección de escritura
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = rd_send_address;
                    }else  if(read_status_code(0x28) && (i == 1)){ // El dato ha sido transmitido y el ACK ha sido recibido
                        i = 0;
                        write_control_register(START_CONDITION); // Start Condition
                        state = rd_setup_slaver_read;
                    } else {
                        write_buffer_usart(0xA8); // COD = 168
                        state = error_I2C;
                    }
                }  
                break;
      
            case rd_setup_slaver_read:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if (read_status_code(0x10)) { // Se lee la repetcion del start condition
                        write_buffer_twi(READ); // Control Byte Read
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = rd_setup_slaver_read;
                    }else if(read_status_code(0x40)){ // El control byte y el ACK de lectura ha sido recibido
                        write_control_register(0xC4); // Se baja la bandera TWINT, sen envia ACK y se mantiene en '1' TWEN
                        state = rd_recieve_data_memory;
                    } else {
                        write_buffer_usart(0xAD); // COD = 173
                        state = error_I2C;
                    }
                 }  
                break;
                
            case rd_recieve_data_memory:
                 if(readFinish()){// Se lee que el TWiNT este en '1'
                    if(read_status_code(0x50)){ // El dato y el ACK se ha recibido
                        data_lsb = read_buffer_twi(); // Se recibe el byte menos significativo del dato a leer
                        write_control_register(0x84); // Se baja la bandera TWINT y se mantiene en '1' TWEN
                        state = rd_stop_slaver;
                    }else{
                        write_buffer_usart(0xB9); // COD = 185
                        state = error_I2C;
                    }
                 }
                break; 
                
            case rd_stop_slaver:
                if(readFinish()){// Se lee que el TWiNT este en '1'
                    if(read_status_code(0x58)){ // El dato y el NACK se ha recibido
                        data_msb = read_buffer_twi(); // Se recibe el byte más significativo del dato a leer
                        write_control_register(0x94); // STOP CONDITION
                        write_buffer_usart(data_lsb); // Se transmite por USART el dato menos significativo del dato leido
                        state = rd_send_data;
                    }else{
                        write_buffer_usart(0xCF); // COD = 207
                        state = error_I2C;
                    }
                }
                break;
                
            case rd_send_data:
                if (tx_flag() && (i == 0)) {
                    i++;
                    tx_flag_down();
                    write_buffer_usart(data_msb); // Se transmite por USART el dato más significativo del dato leido
                    state = rd_send_data;
                }else if(tx_flag() && (i == 1)) {
                    i = 0;
                    write_buffer_usart(0xCB); // finish_rd
                    state = finish_read_data;
                } else {
                    state = rd_send_data;
                }
                break;
      
            case finish_read_data:
                if(tx_flag()){
                    tx_flag_down();
                    end_fsm_read_data = 1;
                    state = read_data;
                }else{
                    state = finish_read_data;
                }
                break;
            /*-------------------------- Error I2C ----------------------------*/
            case error_I2C:
                if (tx_flag()) {
                    tx_flag_down();
                    i = 0;
                    contador_2000 = 0;
                    state = read_function_usart;
                } else {
                    state = error_I2C;
                }
                break;
        }// switch state
    }// while loop
}// main

void setup_function(){
    /*  Setup I2C */
    TWBR = 0xC0; // 192
    TWCR = 0xC5; // I2C Control Register
    /* Setup USART*/
    UCSR0A = 0x20;
    UCSR0B = 0x00; // RXEN0 = 1 | TXEN0 = 0
    UCSR0C = 0x26;
    UBRR0L = 0xA0; // UBRR0 = 416 |  | BAUDRATE = 2400
    UBRR0H = 0x01;
}// setup_function
