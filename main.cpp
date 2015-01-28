#include <kvs/qt/Application>
#include <QApplication>

#include <QtGui>

#include <vector>
#include <string>

#include "mainwindow.h"


int main(int argc, char *argv[])
{
  
  kvs::qt::Application app( argc, argv );
  const int width = 1800;
  const int height = 800;


  MainWindow mainwindow(&app );
  mainwindow.resize( width, height );
  mainwindow.show();

    
  return app.run();
  
}
