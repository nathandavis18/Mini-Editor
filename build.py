from os import system
from sys import argv
def build():
    initialCommand = 'cmake -B ./out/build'
    createRelease = False
    if(len(argv) == 2):
        if(argv[1] == '--build-tests'):
            initialCommand += ' -DBUILD_TESTS=ON -DBUILD_PROJECT=ON'
        elif(argv[1] == '--tests-only'):
            initialCommand += ' -DBUILD_TESTS=ON -DBUILD_PROJECT=OFF'
        elif(argv[1] == '--create-release'):
            initialCommand += ' -DBUILD_PROJECT=ON -DCREATE_RELEASE=ON'
            createRelease = True
        else:
            print("Invalid option. Options include:\n\nNo option: Builds only the project\n--build-tests: Builds project with tests\n--tests-only: Builds only the tests\n")
            exit()
    elif(len(argv) == 1):
        initialCommand += ' -DBUILD_PROJECT=ON'
    else:
        print("Invalid option. Options include:\n\nNo option: Builds only the project\n--build-tests: Builds project with tests\n--tests-only: Builds only the tests\n")
        exit()

    system(initialCommand)
    system('cmake --build ./out/build --config Release')
    if(createRelease):
        system('cd ./out/build && cpack')

if __name__ == '__main__':
    build()