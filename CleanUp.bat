@ECHO OFF

FOR %%i IN (Exe, Output, Settings, Debug, Shipping, Temp) DO IF EXIST %%i RD %%i /S/Q
FOR %%i IN (EX~, DEP, OPT, PLG, APS, NCB, TMP, LOG, ILK, SIO, ERR, TPU, RES, emSession, jlink) DO IF EXIST *.%%i DEL *.%%i