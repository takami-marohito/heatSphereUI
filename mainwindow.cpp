#include "mainwindow.h"

#include <kvs/qt/Screen>
#include <QtGui>


#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include "mainwidget.h"

#include <string.h>

#include <kvs/Directory>
#include <kvs/FileList>
#include <kvs/File>


MainWindow::MainWindow(kvs::qt::Application* app):animation_flag(false),is_animation(false)
 {
   m_mainwidget = new MainWidget(app);
   m_screen = new kvs::qt::Screen( app );
   QMainWindow::setCentralWidget(m_mainwidget);
   
   m_open_action = new QAction( tr("&Open"), this) ;
   QObject::connect( m_open_action, SIGNAL( triggered() ), this, SLOT( open() ) );
   m_open_sei_action = new QAction( tr("&OpenSei"), this) ;
   QObject::connect( m_open_sei_action, SIGNAL( triggered() ), this, SLOT( open_sei() ) );
   m_open_another_time_action = new QAction( tr("&ChangeTime"), this) ;
   QObject::connect( m_open_another_time_action, SIGNAL( triggered() ), this, SLOT( open_another_time() ) );
   m_open_animation_action = new QAction( tr("Open&Animation"), this) ;
   QObject::connect( m_open_animation_action, SIGNAL( triggered() ), this, SLOT( open_animation() ) );
   m_change_th_sei_action = new QAction( tr("ChangeThreashold"), this) ;
   QObject::connect( m_change_th_sei_action, SIGNAL( triggered() ), this, SLOT( change_th_sei() ) );
   m_exit_action = new QAction( tr("&Exit"), this) ;
   QObject::connect( m_exit_action, SIGNAL( triggered() ), this, SLOT( exit() ) );

   m_file_menu = menuBar()->addMenu( tr("&File") );
   m_GradT_menu = menuBar()->addMenu( tr("&GradT") );
   m_SecondInvariant_menu = menuBar()->addMenu( tr("&SecondInvariant") );
   m_file_menu->addAction( m_open_action );
   m_SecondInvariant_menu->addAction( m_change_th_sei_action );
   m_GradT_menu->addAction( m_open_another_time_action );
   m_file_menu->addAction( m_open_sei_action );
   m_file_menu->addAction( m_open_animation_action );
   m_file_menu->addAction( m_exit_action );
 }

MainWindow::~MainWindow()
{
  delete m_screen;
  delete m_mainwidget;
  delete m_open_action;
  delete m_exit_action;
  delete m_file_menu;
}

void MainWindow::open()
{
  QFileDialog* dialog = new QFileDialog( this, "Open file ...", QDir::homePath() );
  if ( dialog->exec() )
    {
      QStringList filenames = dialog->selectedFiles();
      QString filename_first = filenames.at(0);

  QString fn = QFileDialog::getExistingDirectory(this,"Open Dir", QDir::homePath() );
  if( fn!=NULL){
    kvs::Directory directory( fn.toStdString() );
    const kvs::FileList files = directory.fileList();
    int file_length = files.size();
    std::vector<std::string> filename;
    std::vector<std::string> filename_sei;
    int nsteps = 0;
    int sei_steps = 0;
    int first_step = 0;

    for ( int i = 0; i < file_length; i++ ){
      const kvs::File file = files[i];
      if( file.extension() == "dat" ){
	nsteps++;
	filename.push_back( file.filePath() );
	if(filename_first.toStdString() == filename.at(nsteps-1)){
	  first_step = nsteps-1;
	}
      }
      if( file.extension() == "sei" ){
	sei_steps++;
	filename_sei.push_back( file.filePath() );
      }
    }
    if(nsteps != sei_steps){
      printf("%d files detected and %d SecondInvariant files detected\n",nsteps, sei_steps);//54800
      m_mainwidget->opendir(filename, nsteps, first_step);
    }else{
      printf("%d files detected\n",nsteps);//54800
      m_mainwidget->opendir(filename, filename_sei,  nsteps, first_step);
    }
  }
    }
}

void MainWindow::open_sei()
{
  QFileDialog* dialog = new QFileDialog( this, "Open file ...", QDir::homePath() );
  if ( dialog->exec() )
    {
      QStringList filenames = dialog->selectedFiles();
      QString filename = filenames.at(0);
      
      m_mainwidget->open_sei(filename.toStdString());
    }
}

void MainWindow::open_animation()
{
  animation_flag = true;
  QString fn = QFileDialog::getExistingDirectory(this,"Open Dir", QDir::homePath() );

      kvs::Directory directory( fn.toStdString() );
      const kvs::FileList files = directory.fileList();
      int file_length = files.size();

      std::vector<std::string> filename;
      int nsteps = 0;

      for ( int i = 0; i < file_length; i++ ){
	const kvs::File file = files[i];
	if( file.extension() == "kvsml" ){
	  nsteps++;
	  filename.push_back( file.filePath() );
	  printf("%s\n",filename.at(nsteps-1).c_str());
	}
      }
      m_mainwidget->open_animation(filename, nsteps);
  is_animation = true;
  animation_flag = false;
}

void MainWindow::exit()
{
  MainWindow::close();
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{   
  // if(is_animation == true){
   if (event->key() == Qt::Key_S) {
     if(animation_flag == false){
       m_mainwidget->play_animation();
       animation_flag = true;
       printf("a\n");
     }else{
       m_mainwidget->stop_animation();
       animation_flag = false;
     }
   }
   //}
}

void MainWindow::open_another_time()
{
  QFileDialog* dialog = new QFileDialog( this, "Open file ...", QDir::homePath() );
  if ( dialog->exec() )
    {
      QStringList filenames = dialog->selectedFiles();
      QString filename = filenames.at(0);
      
      m_mainwidget->open_another_time(filename.toStdString());
    }
}

void MainWindow::change_th_sei()
{
  bool ok;
  float input = QInputDialog::getDouble(this,tr("Input the Threshold of SecondInvariant"),tr("SecondInvariant:"), 1500, 0, 10000, 1, &ok);
  if ( ok )
    {
      //printf("%f\n",input);
      m_mainwidget->change_th_sei(input);
    }
}
