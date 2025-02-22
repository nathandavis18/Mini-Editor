from os import system
from sys import argv
def build():
    initialCommand = 'cmake -B ./out/build -DCMAKE_BUILD_TYPE=Release'
    if(len(argv) == 2):
        if(argv[1] == '--build-tests'):
            initialCommand += ' -DBUILD_TESTS=On'
    system(initialCommand)
    system('cmake --build ./out/build --config Release')

if __name__ == '__main__':
    build()