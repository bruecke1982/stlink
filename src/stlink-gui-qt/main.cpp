#include "mainwindow.h"
#include <QApplication>
#include <QResource>

QApplication *g_pApp;

int main(int argc, char** argv)
{
   int res;
   //Q_INIT_RESOURCE(application);

   g_pApp = new QApplication(argc, argv);
   QResource::registerResource("resources/resources.rcc");
   MainWindow mainWindow(nullptr);
   mainWindow.show();
   res = g_pApp->exec();
   return res;
}

#ifdef _MSC_VER
#include <windows.h>
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int iShow)
{
   return main(0, 0L);
}
#endif
