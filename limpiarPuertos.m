% se limpia matlab
clear all % se borran las variables
close all % se cierra las ventanas 
clc % se borra el comand windows
% se configura el puerto seria a usar
if ~isempty(instrfind) % se pregunta si hay puertos abiertos
    fclose(instrfind); % se cierran los puertos abiertos
    delete(instrfind); % se borran los puertos abiertos
end