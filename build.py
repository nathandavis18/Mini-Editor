from os import system
def build():
    system('cmake -B ./out/build -DCMAKE_BUILD_TYPE=Release')
    system('cmake --build ./out/build --config Release')

if __name__ == '__main__':
    build()