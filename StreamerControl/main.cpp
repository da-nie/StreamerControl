#include "cmainwindow.h"
#include <QApplication>
#include <QTextCodec>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>

// Глобальные переменные для хранения аргументов командной строки
static int g_argc;
static char** g_argv;

// Проверка, запущена ли программа с правами root
bool check_root() {
    return geteuid() == 0;
}

// Перезапуск программы с sudo, если не root
void restart_as_root() {
    if (!check_root()) {
        std::cout << "Программа требует прав root. Запрашиваю пароль..." << std::endl;

        // Получаем путь к исполняемому файлу
        char exe_path[4096];
        ssize_t len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
        if (len == -1) {
            std::cerr << "Ошибка получения пути к исполняемому файлу" << std::endl;
            std::exit(1);
        }
        exe_path[len] = '\0';

        // Получаем переменные окружения DISPLAY и XAUTHORITY
        std::string display = std::getenv("DISPLAY") ? std::getenv("DISPLAY") : ":0";
        std::string xauthority = std::getenv("XAUTHORITY") ?
            std::getenv("XAUTHORITY") :
            std::string(std::getenv("HOME")) + "/.Xauthority";

        // Формируем команду для pkexec
        std::vector<std::string> args = {
            "pkexec",
            "env",
            "DISPLAY=" + display,
            "XAUTHORITY=" + xauthority,
            exe_path
        };

        // Добавляем аргументы командной строки
        for (int i = 1; i < g_argc; ++i) {
            args.push_back(g_argv[i]);
        }

        // Преобразуем в массив char* для execvp
        std::vector<char*> c_args;
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        // Запускаем команду
        pid_t pid = fork();
        if (pid == -1) {
            std::cerr << "Ошибка при создании процесса" << std::endl;
            std::exit(1);
        }
        else if (pid == 0) {
            // Дочерний процесс
            execvp("pkexec", c_args.data());
            std::cerr << "Не удалось получить права root. Программа завершена." << std::endl;
            std::exit(1);
        }
        else {
            // Родительский процесс
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                std::exit(0);
            }
            else {
                std::cerr << "Не удалось получить права root. Программа завершена." << std::endl;
                std::exit(1);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // Сохраняем аргументы командной строки
    g_argc = argc;
    g_argv = argv;

    // Проверяем и запрашиваем права root
    restart_as_root();

    // Для работы без установленной Qt требуется такая инициализация
    QStringList paths = QCoreApplication::libraryPaths();
    paths.append(".");
    paths.append("platforms");

    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    QApplication qApplication(argc, argv);
    qApplication.setStyle("motif"); // Исправлено: "motiff" -> "motif"
    CMainWindow cMainWindow;
    cMainWindow.show();

    return qApplication.exec();
}
