//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit2.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TProgress *Progress;


class TMyThread : public TThread
{
__published:    // IDE-managed Components
private:    // User declarations
protected:  // User declarations
    void __fastcall Execute();
public:     // User declarations
    __fastcall TMyThread(bool suspended);
};

__fastcall TMyThread::TMyThread(bool suspended)
    : TThread(suspended)
{
}

bool mythreadRunning;
TMyThread *SecondProcess; // TMyThread is a custom descendant of TThread

void __fastcall TMyThread::Execute()
{

        while(!Terminated)
        {
         Progress->ProgressBar1->Min = 0;
         Progress->ProgressBar1->Max = 100;
         //

         for(int i = 0; i < 100; i++)
         {
                Application->ProcessMessages();
                //Sleep(500);
                Progress->ProgressBar1->Position = i;
                if (Terminated)
                        break;

         }
        }
        mythreadRunning = FALSE;
}


//---------------------------------------------------------------------------
__fastcall TProgress::TProgress(TComponent* Owner)
        : TForm(Owner)
{
        mythreadRunning = FALSE;
}
//---------------------------------------------------------------------------
void __fastcall TProgress::FormShow(TObject *Sender)
{

  if (mythreadRunning == FALSE)
  {
    //ProgressBar1->Smooth = true;
    mythreadRunning = TRUE;
    SecondProcess = new TMyThread(True); // create suspended – secondprocess does not run yet
    SecondProcess->FreeOnTerminate = True; // don't need to cleanup after terminate
    SecondProcess->Priority = tpLower;  // set the priority to lower than normal
    SecondProcess->Resume(); // now run the thread
  }
  else
    MessageDlg("This thread is still running.  You are going to hurt yourself!",
      mtInformation, TMsgDlgButtons() << mbOK, 0);

}
//---------------------------------------------------------------------------
void __fastcall TProgress::FormHide(TObject *Sender)
{
        if (mythreadRunning == TRUE)
        {
                SecondProcess->Terminate();
                mythreadRunning = FALSE;
        }

}
//---------------------------------------------------------------------------

