
只能拷贝到命令提示符中执行
for /R .\ %f in (*.h) do astyle --style=allman "%f"
for /R .\ %f in (*.c) do astyle --style=allman "%f"
rm **/*.orig