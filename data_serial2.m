function [data] = data_serial2(s)
    data = 0;
    for i = 1:1000
        if s.NumBytesAvailable ~= 0
            data = read(s,1,"uint8"); %% Finish RD
            if(data == 49)
                disp('Temperatura');
            end
            break;
        end
    end
end