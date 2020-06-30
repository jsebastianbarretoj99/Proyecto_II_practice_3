% se limpia matlab
clear all % se borran las variables
close all % se cierra las ventanas 
clc % se borra el comand windows
% se configura el puerto seria a usar
if ~isempty(instrfind) % se pregunta si hay puertos abiertos
    fclose(instrfind); % se cierran los puertos abiertos
    delete(instrfind); % se borran los puertos abiertos
end
s = serialport("COM3", 2400,"Parity", "even", "DataBits" , 8, "StopBits", 1, "TimeOut", 2);% se configura el puerto para ser leido serialmente a 2400bps
start = true;
while(start)
    prompt = 'Select function: ';
    select_function = input(prompt,'s');
    switch select_function
        case 'Write_1000'
            disp('Write_1000')
            signal_ena = true;
            while(signal_ena)
                prompt = 'Select Signal: ';
                select_signal = input(prompt,'s');
                switch select_signal
                    case 'Cuadrada'
                        datos = csvread('datos_cuadrada.csv');
                        signal_ena = false;
                    
                    case 'Diente'
                        datos = csvread('datos_diente_sierra.csv'); 
                        signal_ena = false;
                    case 'Seno'
                        datos = csvread('datos_1000.csv');
                        signal_ena = false;
                end
            end
            write(s,'W',"char");
            ready_w1 = data_serial(s,0); %% se lee respuesta inicial W1 = 170
            if( ready_w1 == 170)
                disp('ready_w1')
            end
            for i = 1:length(datos)
                lsb = datos(i) - floor(datos(i)/2^(8))*2^(8);
                msb = floor(datos(i)/2^(8));
                write(s,lsb,"uint8");% LSB
                write(s,msb,"uint8"); % MSB
                data_serial(s,1); % se lee el error
                if i == length(datos)
                    write(s,'S',"char"); %Continue
                    finish_w1 = read(s,1,"uint8"); % se lee respuesta final W1 = 171
                    if( finish_w1 == 171)
                        disp('finish_w1')
                    end
                else
                    write(s,'C',"char"); %Continue
                end
            end
            
        case 'Read_1000'
            L = 1001;
            disp('Read_1000')
            write(s,'R',"char");
            ready_r1 = data_serial(s,0); %% ready R1 186
            if( ready_r1 == 186)
                disp('ready_r1')
            end
            t = 1:1:L;
            recieve_msb = zeros(L,1);
            recieve_lsb = zeros(L,1);
            receive_data = zeros(L,1);
            for i = 1:1:L
                recieve_lsb(i) = data_serial(s,1); %% Dato MSB || Error
                if(~(recieve_lsb(i) == 1 || recieve_lsb(i) == 25 || recieve_lsb(i) == 27 || recieve_lsb(i) == 48 || recieve_lsb(i) == 70 || recieve_lsb(i) == 82 || recieve_lsb(i) == 84 || recieve_lsb(i) == 87 || recieve_lsb(i) == 101 || recieve_lsb(i) == 168 || recieve_lsb(i) == 173 || recieve_lsb(i) == 185 || recieve_lsb(i) == 207))
                    recieve_msb(i) = data_serial(s,0); %% Dato LSB
                    receive_data(i) = recieve_msb(i)*2^(8) + recieve_lsb(i);
                end
                if i == L
                    write(s,'S',"char"); %Continue
                    finish_r1 = read(s,1,"uint8"); % se lee respuesta final R1 = 187
                    if(finish_r1 == 187)
                        disp('finish_r1');
                    end
                else
                    write(s,'C',"char"); %Continue
                end
            end
            plot(t,receive_data,'m','LineWidth',1.5)
            title('Información Guardada CSJ')
            grid on
            axis([0 1001 -1000 70000])
                
        case 'Read_data'
            disp('Read_data')
            write(s,'D',"char");
            ready_rd = data_serial(s,1); %% ready
            if( ready_rd == 202)
                disp('ready_rd')
            end
            prompt = 'Seleccion dato lectura: ';
            dato = input(prompt);
            addres = dato*2;
            a_msb = floor(addres/2^(8));
            a_lsb = addres - floor(addres/2^(8))*2^(8);
            write(s,a_lsb,"uint8"); %% Address LSB
            write(s,a_msb,"uint8"); %% Address MSB
            re_lsb = data_serial(s,1); %% lsb
            if(~(re_lsb == 1 || re_lsb == 25 || re_lsb == 27 || re_lsb == 48 || re_lsb == 70 || re_lsb == 82 || re_lsb == 84 || re_lsb == 87 || re_lsb == 101 || re_lsb == 168 || re_lsb == 173 || re_lsb == 185 || re_lsb == 207))
                re_msb = data_serial(s,1); %% msb
                finish_rd = data_serial(s,1); %% finish
                receive_d = re_msb*2^(8) + re_lsb;
                formatSpec = 'X is %4.2f meters or %8.3f mm\n';
                texto = ['Dato recibido = ',num2str(receive_d)];
                disp(texto)
            end
            if( finish_rd == 203)
                disp('finish_rd')
            end
            
        case 'Exit'
            disp('Exit')
            start = false;
            
        otherwise
            disp('Invalid Function')
    end
end
clear all % se borran las variables
close all % se cierra las ventanas 
clc % se borra el comand windows
if ~isempty(instrfind) % se pregunta si hay puertos abiertos
    fclose(instrfind); % se cierran los puertos abiertos
    delete(instrfind); % se borran los puertos abiertos
end