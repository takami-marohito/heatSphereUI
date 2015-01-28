#ifndef RTHETAPHI_H
#define RTHETAPHI_H

#include <QWidget>
#include <QSlider>

#include "toolwidget.h"


class RthetaphiSlider:public QWidget
{
  Q_OBJECT

  private:
  ToolWidget *m_toolwidget;
  QSlider* slider_r;
  QSlider* slider_theta;
  QSlider* slider_phi;
  QPushButton* bapply;
  int mode;

  public:
  RthetaphiSlider(QWidget *parent);
  ~RthetaphiSlider();

  private slots:
  void ChangeSlider_r(int parameter);
  void ChangeSlider_theta(int parameter);
  void ChangeSlider_phi(int parameter);
  void apply();
};
#endif
