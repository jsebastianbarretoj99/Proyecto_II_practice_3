function [data] = data_serial(s, sel)
    data = 0;
    for i = 1:1000
        if s.NumBytesAvailable ~= 0
            data = read(s,1,"uint8"); %% Finish RD
            if(sel == 1 && (data == 1 || data == 25 || data == 27 || data == 48 || data == 70 || data == 82 || data == 84 || data == 87 || data == 101 || data == 168 || data == 173 || data == 185 || data == 207))
                disp(data);
            end
            break;
        end
    end
end