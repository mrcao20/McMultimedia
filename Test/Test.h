#pragma once
#pragma execution_character_set("utf-8")

#include <QtWidgets/QMainWindow>
#include "ui_Test.h"

class McMediaPlayer;

class Test : public QMainWindow
{
	Q_OBJECT

public:
	Test(QWidget *parent = Q_NULLPTR);

protected:
	bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
	Ui::TestClass ui;

	McMediaPlayer *m_mediaPlayer;
};
