#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <array>
#include <vector>
#include <QMainWindow>
#include <QFuture>
#include "stlink.h"

#define WORKAROUND
class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QGroupBox;
class QLabel;
class QPlainTextEdit;
class QTimer;

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
   void slotTimerProgressElapsed();
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
   QTimer         *m_pTimerProgress        = nullptr;
   QFuture<int>    m_QFutureWait;
   QByteArray      m_aBinaryData;
   virtual void enbaleForms(bool value);
   virtual void refreshGroupBoxDeviceInfo();
   virtual void clearGroupBoxDeviceInfo();
   virtual void updateTextWithTime(int color, const QString &str);
   virtual void updateText(int color, const QString &str);
   virtual void updateTextSameLine(int color, const QString &str);
   virtual void flashWorker();
   virtual void progressPoints(uint32_t count, const QString &starttext = "");
private:
   stlink_t *m_pStlinkHandle = nullptr;
   QString m_sLastDirectory = "/home";
   enum class Progress
   {
      START_WRITE_FLASH,
      WAIT_WRITE_FLASH,
      FINISHED_WRITE_FLASH,
      START_VERIFY_FLASH,
      WAIT_VERIFY_FLASH,
      FINISHED_VERIFY_FLASH,
      FINISHED
   };
   Progress m_Progress = Progress::START_WRITE_FLASH;
   uint32_t m_uStartAddress = 0;
   uint32_t m_uProgressCounter = 0;
};

#endif // MAINWINDOW_H_
