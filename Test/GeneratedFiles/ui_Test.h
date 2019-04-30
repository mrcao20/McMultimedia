/********************************************************************************
** Form generated from reading UI file 'Test.ui'
**
** Created by: Qt User Interface Compiler version 5.9.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TEST_H
#define UI_TEST_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "mcopenglrenderer.h"

QT_BEGIN_NAMESPACE

class Ui_TestClass
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    McOpenGLRenderer *videoOutput;
    QSlider *progressBar;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *stopBtn;
    QPushButton *backwardBtn;
    QPushButton *playBtn;
    QPushButton *forwardBtn;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_2;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *TestClass)
    {
        if (TestClass->objectName().isEmpty())
            TestClass->setObjectName(QStringLiteral("TestClass"));
        TestClass->resize(813, 564);
        centralWidget = new QWidget(TestClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        videoOutput = new McOpenGLRenderer(centralWidget);
        videoOutput->setObjectName(QStringLiteral("videoOutput"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(videoOutput->sizePolicy().hasHeightForWidth());
        videoOutput->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(videoOutput);

        progressBar = new QSlider(centralWidget);
        progressBar->setObjectName(QStringLiteral("progressBar"));
        progressBar->setOrientation(Qt::Horizontal);

        verticalLayout->addWidget(progressBar);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        stopBtn = new QPushButton(centralWidget);
        stopBtn->setObjectName(QStringLiteral("stopBtn"));
        stopBtn->setMinimumSize(QSize(40, 40));
        stopBtn->setMaximumSize(QSize(40, 40));

        horizontalLayout->addWidget(stopBtn);

        backwardBtn = new QPushButton(centralWidget);
        backwardBtn->setObjectName(QStringLiteral("backwardBtn"));
        backwardBtn->setMinimumSize(QSize(40, 40));
        backwardBtn->setMaximumSize(QSize(40, 40));

        horizontalLayout->addWidget(backwardBtn);

        playBtn = new QPushButton(centralWidget);
        playBtn->setObjectName(QStringLiteral("playBtn"));
        playBtn->setMinimumSize(QSize(40, 40));
        playBtn->setMaximumSize(QSize(40, 40));

        horizontalLayout->addWidget(playBtn);

        forwardBtn = new QPushButton(centralWidget);
        forwardBtn->setObjectName(QStringLiteral("forwardBtn"));
        forwardBtn->setMinimumSize(QSize(40, 40));
        forwardBtn->setMaximumSize(QSize(40, 40));

        horizontalLayout->addWidget(forwardBtn);

        pushButton = new QPushButton(centralWidget);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        horizontalLayout->addWidget(pushButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout);

        TestClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(TestClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 813, 23));
        TestClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(TestClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        TestClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(TestClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        TestClass->setStatusBar(statusBar);

        retranslateUi(TestClass);

        QMetaObject::connectSlotsByName(TestClass);
    } // setupUi

    void retranslateUi(QMainWindow *TestClass)
    {
        TestClass->setWindowTitle(QApplication::translate("TestClass", "Test", Q_NULLPTR));
        stopBtn->setText(QApplication::translate("TestClass", "stop", Q_NULLPTR));
        backwardBtn->setText(QApplication::translate("TestClass", "back", Q_NULLPTR));
        playBtn->setText(QApplication::translate("TestClass", "play", Q_NULLPTR));
        forwardBtn->setText(QApplication::translate("TestClass", "Forward", Q_NULLPTR));
        pushButton->setText(QApplication::translate("TestClass", "PushButton", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class TestClass: public Ui_TestClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TEST_H
