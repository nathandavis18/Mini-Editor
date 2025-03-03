from os import system
from sys import argv
def build():
    initialCommand = 'cmake -B ./out/build'
    if(len(argv) == 2):
        if(argv[1] == '--build-tests'):
            initialCommand += ' -DBUILD_TESTS=ON'
        elif(argv[1] == '--tests-only'):
            initialCommand += ' -DBUILD_TESTS=ON -DBUILD_PROJECT=OFF'
        else:
            print("Invalid option. Options include:\n\nNo option: Builds only the project\n--build-tests: Builds project with tests\n--tests-only: Builds only the tests\n")
            exit()

    system(initialCommand)
    system('cmake --build ./out/build --config Release')

if __name__ == '__main__':
    build()