@echo off

echo =========================
echo Building MSI...
echo =========================

"%WIX%\bin\candle.exe" Product.wxs
if %errorlevel% neq 0 pause

"%WIX%\bin\light.exe" Product.wixobj -ext WixUIExtension -o Lauchwatch.msi
if %errorlevel% neq 0 pause

echo =========================
echo DONE! MSI erstellt 🎉
echo =========================

pause