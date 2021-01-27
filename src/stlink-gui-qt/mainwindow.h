#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <array>
#include <vector>
#include <QMainWindow>
#include "stlink.h"

#define WORKAROUND
class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QLabel;
class QPlainTextEdit;

class MainWindow : public QMainWindow
{
   Q_OBJECT

public:
   MainWindow(QMainWindow *pParent = nullptr);
   virtual ~MainWindow();

protected slots:
   void slotButtonLoadFile();
   void slotButtonConnect();
   void slotButtonDisconnect();
   void slotButtonFlash();
protected:
   void closeEvent(QCloseEvent *bar) override;
   void initGui();
   QGroupBox      *m_pGroupBoxDeviceInfo   = nullptr;
   QLabel         *m_pLabelChip            = nullptr;
   QLabel         *m_pLabelCore            = nullptr;
   QLabel         *m_pLabelFlashSize       = nullptr;
   QLabel         *m_pLabelRamSize         = nullptr;
   QPushButton    *m_pButtonLoadFile       = nullptr;
   QPushButton    *m_pButtonConnect        = nullptr;
   QPushButton    *m_pButtonDisconnect     = nullptr;
   QPushButton    *m_pButtonFlash          = nullptr;
   QPlainTextEdit *m_pPlainTextOutput      = nullptr;
   QLineEdit      *m_pLineEditStartAddress = nullptr;
   QLineEdit      *m_pLineEditFilePath     = nullptr;
   QByteArray      m_aBinaryData;
   virtual void enbaleForms(bool value);
   virtual void refreshGroupBoxDeviceInfo();
   virtual void clearGroupBoxDeviceInfo();
   virtual void updateTextWithTime(int color, const QString &str);
   virtual void updateText(int color, const QString &str);
   virtual void flashWorker();
private:
   stlink_t *m_pStlinkHandle = nullptr;
   QString m_sLastDirectory = "/home";
};

#endif // MAINWINDOW_H_
