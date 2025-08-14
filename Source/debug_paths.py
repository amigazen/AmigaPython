import sys; print('=== PATH DEBUGGING ==='); print('sys.path:'); [print('  ' + repr(p)) for p in sys.path]; print('
sys.prefix:', repr(sys.prefix)); print('sys.exec_prefix:', repr(sys.exec_prefix)); print('
=== FILE CHECKING ==='); import os; print('os.path.exists("/Python/Lib"):', os.path.exists('/Python/Lib')); print('os.path.exists("/Python/Lib/site.py"):', os.path.exists('/Python/Lib/site.py')); print('os.path.exists("/Python/Lib/zipimport.py"):', os.path.exists('/Python/Lib/zipimport.py')); print('
=== DIRECTORY LISTING ==='); try: print('Files in /Python/Lib:'); [print('  ' + f) for f in os.listdir('/Python/Lib')[:10]]; except Exception as e: print('Error listing /Python/Lib:', e)
