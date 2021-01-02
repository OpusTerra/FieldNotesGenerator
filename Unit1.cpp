//---------------------------------------------------------------------------


// Ligne 339 31 mars
// 29 Avril 09: 1.09c Ajout de code oublié dans le cas où on a pas de Time Found et vitesse 2
// 16 Aout 2010: Ajout "" a la fin d'un record de DNF dasn le fieldnotes file
// 20 Aout 2014: Changé .GDB avant .GPX dans browse for GPS File
// Aussi diminué   MAX_GPXFILESIZE car Out of Memory au lInker
#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "Unit2.h"
#include "Unit3.h"
#include <IniFiles.hpp>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <iostream>
#include <math.h>
#include <excpt.h>
#include <values.h>
#include "Coord.h"
#include <wchar.h>
#include <stddef.h>
#include <fcntl.h>
#include <process.h>
#include <vcl\Clipbrd.hpp>

using namespace std;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
AnsiString LabelMessage;
int ColumnToSort = 0;
int ErrorCallingBabel = 0;
int DNFCounter;

#define MAX_CachesFound 2000
#define MAX_TrackSegments 200000   // Était à 60000 et Changé à 200000    27 Mai 09
//#define MAX_GPXFILESIZE 99999999          // le max d'une std::string est de 4294967281
#define MAX_GPXFILESIZE 44999999          // le max d'une std::string est de 4294967281


int sort_function( const void *a, const void *b);
int TrkSsort_function( const void *a, const void *b);

DWORD CreateProcessEx ( LPCSTR lpAppPath, LPCSTR lpCmdLine, BOOL bWaitForProcess, BOOL bMinimizeOnWait, HWND hMainWnd );
HRESULT __fastcall AnsiToUnicode(LPCSTR pszA, LPOLESTR* ppszW);
HRESULT __fastcall UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA);

char buffer[MAX_GPXFILESIZE];
int TOOFAR_DISTANCE = 150; // in meters
double DoubleTrackTimeRatio; // expressed as # minutes / 1440 per day
double MinDoubleTrackDist;


typedef struct {
        char GeocacheCode[30];
        double Latitude;
        double Longitude;
        double Distance;  char TimeFound[25];
        double Distance2; char TimeFound2[25]; // if there is a second pass near this geocache at another time
        long trkseg;
        long trkseg2;
        int CacheType; // 0=Any 1=Found or 2=DNF or eventually something else ?
} t_cachefound;

t_cachefound CachesFound[MAX_CachesFound];

typedef struct {
        double Latitude;
        double Longitude;
        char TimeSegment[25];
        double vitesse;  // en m/sec
} t_trkseg;

t_trkseg TrackSegments[MAX_TrackSegments];

int EditFile(char *s);

char CurrentOutFileName[8192];

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
        ColumnToSort = 0;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button1Click(TObject *Sender)
{

  OpenDialog1->Options.Clear();
  OpenDialog1->Options << ofFileMustExist;
  //OpenDialog1->Filter = "GPX files (*.gpx)|*.GPX|GDB files (*.gdb)|*.gdb|All files (*.*)|*.*";
  OpenDialog1->Filter = "GDB files (*.gdb)|*.gdb|GPX files (*.gpx)|*.GPX|All files (*.*)|*.*";
                         
  // OpenDialog1->FilterIndex = 1; // start the dialog showing all files
  if (OpenDialog1->Execute())
  {
      GPXFile->Clear();
      GPXFile->SelText = OpenDialog1->Files->Strings[0].c_str();
  }

}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void __fastcall TForm1::ProcessClick(TObject *Sender)
{
        FILE *fpGPX, *fpNotes, *fpCodeList;
        int i, j, k;
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
        char OutputNotes[_MAX_PATH];
        size_t pos, pos2, pos3;
        int FoundCachesCounter;
        string StringFound = "<sym>Geocache Found</sym>";
        string StringAnyCache = "<sym>Geocache";
        string StringDNF;
        string StringCache;
        string CacheCodeDesc;
        string CacheLatLon = "<wpt lat=";
        string TraskSegStart = "<trkpt lat=";
        string TimeStart = "<time>";
        string w,x;
        char GeocacheCode[30];
        char GeocacheCodeTemp[30];
        char LatLon[100];
        char *c, *c2;
        int TrackSegmentCounter;
        CLatLon Point1, Point2;
        double dFwdAz, dRevAz;
        double dDistance;
        char ibf[8192];
        LPOLESTR pszOutStringW;
        string gpx;
        time_t     now;
        struct tm  *ts, tf;
        char       buf[80];
        SYSTEMTIME sUTCTime, sLocalTime;
        TListItem  *ListItem;
        int handle;
        unsigned long GPXFileLength, gpxstringmaxsize;
        int TooFarCachesCtr = 0;
        TDateTime dt1, dt2;
        AnsiString s;
        double tdiff;
        char TimeFoundXchange[25];
        int buggy;
        long t1, t2;
        TCursor Save_Cursor;
        int result;
        int FieldNotesCounter;
        char    bufTime[80];
        char    bufCounter[80];
        char    bufPartialCounter[80];
        int     CacheCounter = 0;
        int     MultiPass;
        int DiscardBadGC;

        DNFCounter = 0;

        if (access(GPXFile->Text.c_str(),0) != 0)
        {
                MessageDlg("Cannot access this GPS file.",mtError,TMsgDlgButtons() << mbOK,0);
                return;
        }

        _splitpath(GPXFile->Text.c_str(), drive, dir, fname, ext );

        if(stricmp(ext,".gdb") == 0)
        {

            if (access(Form3->GPSBabelPath->Text.c_str(),0) != 0)
            {
                MessageDlg("Cannot access GPS Babel. Please correct GPS Babel path in the parameters screen.",mtError,TMsgDlgButtons() << mbOK,0);
                return;
            }

             // gpsbabel.exe -p "" -w -i gdb -f "C:\Garmin\Geocache\temp\Untitled.gdb" -o gpx -F "C:\Garmin\Geocache\temp\Untitled.gpx"
            Save_Cursor = Screen->Cursor;
            Screen->Cursor = crHourGlass;    // Show hourglass cursor
            _makepath( OutputNotes, drive, dir, "Temp", ".gpx" );

            sprintf(ibf,"\"%s\" -p \"\" -w -i gdb -f \"%s\" -o gpx -F \"%s\"",
                Form3->GPSBabelPath->Text.c_str(),GPXFile->Text.c_str(),OutputNotes);

            Progress->Show();

            result = CreateProcessEx (
                 //"C:\\Garmin\\Geocache\\C++\\AlphaVal\\AlphaVal.exe", // pour testing
                 Form3->GPSBabelPath->Text.c_str(),
                 ibf,
                 True,
                 False,
                 NULL );

             Progress->Hide();
             Screen->Cursor = Save_Cursor; // always restore the cursor
             if (result != 0)
             {
                MessageDlg("Error calling GPS Babel",mtError,TMsgDlgButtons() << mbOK,0);
                return;
              }
              else
                GPXFile->Text = OutputNotes;
        }

        handle = open(GPXFile->Text.c_str(), O_RDONLY);
        //gpxstringmaxsize = gpx.max_size();
        GPXFileLength = filelength(handle);
        close(handle);

        TOOFAR_DISTANCE = atoi(Form3->Tolerance->Text.c_str());
        if(TOOFAR_DISTANCE < 4)
        {
            Application->MessageBox("Tolerance is too small.","Field Notes Gen",MB_OK);
            return;
        }

        // transformer ce timimg exprime en minutes vers une fraction de la journee
        DoubleTrackTimeRatio = (double)atoi(Form3->DoubleTrackTime->Text.c_str()) / 1440;
        MinDoubleTrackDist = (double)atoi(Form3->DoubleTrackDist->Text.c_str());

        if (GPXFileLength > MAX_GPXFILESIZE)
        {
            Application->MessageBox("GPS File size is too big!","Field Notes Gen",MB_OK);
            return;
        }


        Save_Cursor = Screen->Cursor;
        Process->Enabled = False;
        Button1->Enabled = False;
        GMapViewAll->Enabled = False;

        ListView1->Items->Clear();
        Label2->Visible = True;
        Label2->Caption = "";

        OutFileName->Caption = "";

        Application->ProcessMessages();

        if((fpGPX = fopen(GPXFile->Text.c_str(),"rt")) != NULL)
        {
                Screen->Cursor = crHourGlass;    // Show hourglass cursor

	        fread(buffer,sizeof(char),MAX_GPXFILESIZE,fpGPX);
                fclose(fpGPX);

                gpx.clear();
                ZeroMemory(TrackSegments, sizeof(TrackSegments));
                ZeroMemory(CachesFound, sizeof(CachesFound));

                gpx = buffer;
                _splitpath(GPXFile->Text.c_str(), drive, dir, fname, ext );
                strcpy(fname,"FieldNotes_");

                // Get the current time
                now = time(NULL);

                // Format and print the time, "ddd yyyy-mm-dd hh:mm:ss zzz"
                ts = localtime(&now);
                // now format time as this example 20081109170429.txt
                strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", ts);
                strcat(fname,buf);

                _makepath( OutputNotes, drive, dir, fname, ".txt" );

                // batir un array des geocaches found
                FoundCachesCounter = 0;
                pos = 0;
                buggy = 0;
                DiscardBadGC = 0;

                // fixer GC Code style selon Form3->GeocachingCodePosition->ItemIndex
                switch (Form3->GeocachingCodePosition->ItemIndex)
                {
                        case 0:
                                CacheCodeDesc = "<desc>GC";
                                break;
                        case 1:
                                CacheCodeDesc = "<desc>";
                                break;
                        case 2:
                                CacheCodeDesc = "<name>GC";
                                break;
                        case 3:
                                CacheCodeDesc = "<name>";
                                break;
                }

                MultiPass = 1;
                if ( Form3->GCCodeListGenerate->Checked)
                        StringCache = StringAnyCache;
                else
                        StringCache = StringFound;
                        
                StringDNF = Form3->DNFIcon->Text.c_str();

                // Added in Version 1.08b car il y avait un trailing blank à la fin des custom 0-9 mais pas les custom 10-15
                // qqchose comme raccourcir()
                size_t endpos = StringDNF.find_last_not_of(" \t"); // Find the first character position from reverse af
                if( string::npos != endpos )
                        StringDNF = StringDNF.substr( 0, endpos+1 );

                // si DNF demande mais pas en mode all
                if(Form3->DNFChecking->Checked && !Form3->GCCodeListGenerate->Checked)
                       MultiPass = 2;


                while(MultiPass)
                {
                  while ((pos = gpx.find(StringCache,pos)) != string::npos)
                  {
                    // reperer le GC code de la cache
                    pos2 = gpx.rfind(CacheCodeDesc,pos);
                    if(pos2 != string::npos)
                    {
                        char *c;
                        GeocacheCode[0] = '\0';
                        if (Form3->GeocachingCodePosition->ItemIndex == 1 || Form3->GeocachingCodePosition->ItemIndex == 3)
                                strcat(GeocacheCode,"GC");

                        gpx.copy(GeocacheCodeTemp,10,pos2 + 6);  // pour tous les cas on copie plus de caracteres qu'il en faut pour commencer

                        strcat(GeocacheCode,GeocacheCodeTemp);

                        c = strstr(GeocacheCode,"-"); // puis on va chercher a tronquer au delimiteur
                        if(c)
                                *c = '\0';
                        else
                        {
                                c = strstr(GeocacheCode," ");   // ou au premier espace
                                if(c)
                                  *c = '\0';
                                /* D.L Essai 31 mars pour traiter les cas de GC codes sans rien d'autre comme texte
                                else
                                {
                                   buggy = 1;
                                   DiscardBadGC = 1;
                                }
                                */
                                c = strstr(GeocacheCode,"<");
                                if(c)
                                  *c = '\0';
                        }
                        c = strstr(GeocacheCode," ");   // Raccourcir
                        if(c)
                          *c = '\0';

                        strcpy(CachesFound[FoundCachesCounter].GeocacheCode,GeocacheCode);
                        if (strncmpi(GeocacheCode,"GCGC",4) == 0)
                        {
                               buggy = 2;
                               DiscardBadGC = 1;
                        }

                    }
                    else
                    {
                          buggy = 1;
                          DiscardBadGC = 1;
                    }

                    /* Retire de cet endroit V1.09 car ]ca buggait aussitot que ca rencontrait pas le GC code format qu'on cherchait
                    if(buggy)
                    {
                         Screen->Cursor = Save_Cursor; // always restore the cursor
                         Process->Enabled = True;
                         Button1->Enabled = True;
                         gpx.clear();
                         GMapViewAll->Enabled = True;
                         if (buggy == 1)
                                MessageDlg("Sorry, I am having some problems locating GC code.\nPlease check your Geocaching Code Position choice in the parameters.",mtError,TMsgDlgButtons() << mbOK,0);
                         else
                                MessageDlg("Oups! It appears that you have a wrong Geocaching Code Position choice in the parameters.\nPlease select the appropriate type.",mtError,TMsgDlgButtons() << mbOK,0);
                         return;

                    }
                    */
                    if (DiscardBadGC == 0)
                    {

                     // enregistrer la latitude et la longitude de la cache pour eventuellement determiner la distance
                     pos2 = gpx.rfind(CacheLatLon,pos);
                     if(pos2 != string::npos)
                     {
                        gpx.copy(LatLon,50,pos2 + 10);  // 1 more for the "
                        c = strstr(LatLon,"\"");
                        *c = '\0';
                        CachesFound[FoundCachesCounter].Latitude = atof(LatLon);
                        *c = '\"';
                        c = strstr(LatLon,"lon=\"")+5;
                        *strstr(c,"\"") = '\0';
                        CachesFound[FoundCachesCounter].Longitude = atof(c);
                     }
                     CachesFound[FoundCachesCounter].CacheType = 1;    // Type Default to Found Cache
                     if(StringCache == StringAnyCache)
                        CachesFound[FoundCachesCounter].CacheType = 0;
                     else if(StringCache == StringDNF)
                     {
                        CachesFound[FoundCachesCounter].CacheType = 2;
                        DNFCounter++;
                     }

                     FoundCachesCounter++;

                     LabelMessage.sprintf("Reading %s",GeocacheCode);
                     Label2->Caption = LabelMessage;
                    }
                    else     // on laisse tomber ce bad code
                        DiscardBadGC = 0;

                    Application->ProcessMessages();


                    pos = pos+1; // pour aller chercher la prochaine instance de la string cherchee
                  } // end of while ((pos = gpx.find ...

                  MultiPass--;
                  if(MultiPass && Form3->DNFChecking->Checked && !Form3->GCCodeListGenerate->Checked)
                        StringCache = StringDNF;
                } // end while(MultiPass)

                // messages dans le cas ou plus rien de va ...
                if (FoundCachesCounter == 0)
                {
                    if(buggy)
                    {
                         Screen->Cursor = Save_Cursor; // always restore the cursor
                         Process->Enabled = True;
                         Button1->Enabled = True;
                         gpx.clear();
                         GMapViewAll->Enabled = True;
                         if (buggy == 1)
                                MessageDlg("Sorry, I am having some problems locating GC code.\nPlease check your Geocaching Code Position choice in the parameters.",mtError,TMsgDlgButtons() << mbOK,0);
                         else
                                MessageDlg("Oups! It appears that you have a wrong Geocaching Code Position choice in the parameters.\nPlease select the appropriate type.",mtError,TMsgDlgButtons() << mbOK,0);
                         return;

                    }

                }


                fpCodeList = NULL;
                if(Form3->GCCodeListGenerate->Checked)
                {
                     fpCodeList = fopen(Form3->GCCodeListFilename->Text.c_str(),"wb");
                     if(fpCodeList == NULL)
                     {
                        MessageDlg("Problem generating Code List output file.",mtError,TMsgDlgButtons() << mbOK,0);
                        goto dewouar;
                     }
                     else
                     {
                         for(i = 0; i < FoundCachesCounter; i++)
                         {
                                fprintf(fpCodeList,"%s;",CachesFound[i].GeocacheCode);
                         }
                         fclose(fpCodeList);
                         if (FoundCachesCounter)
                         {
                             LabelMessage.sprintf("Fully successful!!!\n%d GC Code were extracted.\nDo you want  to take a look at it?",FoundCachesCounter);
                             if (MessageDlg(LabelMessage,mtInformation,TMsgDlgButtons() <<  mbYes << mbNo,0) == mrYes)
                                EditFile(Form3->GCCodeListFilename->Text.c_str());
                         }
                         else
                         {
                             LabelMessage.sprintf("Oups!\n%d GC Code was extracted.",FoundCachesCounter);
                             MessageDlg(LabelMessage,mtWarning,TMsgDlgButtons() << mbOK,0);
                         }

                     }
                     goto dewouar;
                }



                // trouver la meilleure heure pour chaque cache trouvee a partir des tracks du fichier GPX
                // Batir un array de tout les track segment peu importe la sequence a laquelle ils appartiennent
                TrackSegmentCounter = 0;
                pos = 0;
                while ((pos = gpx.find(TraskSegStart,pos)) != string::npos)
                {
                        gpx.copy(LatLon,50,pos + 12);    // 1 more for the "
                        c = strstr(LatLon,"\"");
                        *c = '\0';
                        TrackSegments[TrackSegmentCounter].Latitude = atof(LatLon);
                        *c = '\"';
                        c = strstr(LatLon,"lon=\"")+5;
                        *strstr(c,"\"") = '\0';
                        TrackSegments[TrackSegmentCounter].Longitude = atof(c);
                        pos2 = gpx.find(TimeStart,pos);
                        if (pos2 == string::npos)
                        {
                                Screen->Cursor = Save_Cursor; // always restore the cursor
                                Process->Enabled = True;
                                Button1->Enabled = True;
                                gpx.clear();
                                GMapViewAll->Enabled = True;
                                MessageDlg("Sorry, but it appears that your track log does not contain any time value (timestamp).\n Consequently, this application will not be able to process anything further.",mtError,TMsgDlgButtons() << mbOK,0);
                                return;
                        }

                        gpx.copy(buffer,50,pos2 + 6);
                        *strstr(buffer,"</time>") = '\0';
                        strcpy(TrackSegments[TrackSegmentCounter].TimeSegment,buffer);

                        if (TrackSegmentCounter < MAX_TrackSegments)
                                TrackSegmentCounter++;
                        else
                                Application->MessageBox("Too Many Track Segments!","Field Notes Gen",MB_OK);

                        LabelMessage.sprintf("Reading Track segment %6d",TrackSegmentCounter);
                        Label2->Caption = LabelMessage;
                        Application->ProcessMessages();

                        pos = pos+1; // pour aller chercher la prochaine instance de la string cherchee
                }

                #ifdef JACK
                FILE * aa;
                aa = fopen("c:\\test1.txt","wt");

                for (j = 0; j <  TrackSegmentCounter; j++)
                {
                        fprintf(aa,"%06ld: %.15lf, %.15lf, %s\n",j,TrackSegments[j].Latitude,
                                TrackSegments[j].Longitude,
                                TrackSegments[j].TimeSegment);

                }
                fclose(aa);

                // Trier les trask segments pour remmetre ensembles
                qsort((void *)TrackSegments, TrackSegmentCounter, sizeof(t_trkseg), TrkSsort_function);

                aa = fopen("c:\\test2.txt","wt");

                for (j = 0; j <  TrackSegmentCounter; j++)
                {
                        fprintf(aa,"%06ld: %.15lf, %.15lf, %s\n",j,TrackSegments[j].Latitude,
                                TrackSegments[j].Longitude,
                                TrackSegments[j].TimeSegment);

                }
                fclose(aa);
                #endif

                // determiner la vitesse à chaque segment de trace
                for (j = 0; j <  TrackSegmentCounter-1; j++)
                {
                        Point1.m_Latitude = TrackSegments[j].Latitude;
                        Point1.m_Longitude = TrackSegments[j].Longitude;

                        Point2.m_Latitude = TrackSegments[j+1].Latitude;
                        Point2.m_Longitude = TrackSegments[j+1].Longitude;

                        // Metric distance evaluation
                        dDistance = Point1.VincentyDistance(Point2, &dFwdAz, &dRevAz);

                        w = TrackSegments[j].TimeSegment;
                        t1 = (atoi(w.substr(11,2).c_str()) * 3600) + (atoi(w.substr(14,2).c_str()) * 60) + atoi(w.substr(17,2).c_str());
                        w = TrackSegments[j+1].TimeSegment;
                        t2 = (atoi(w.substr(11,2).c_str()) * 3600) + (atoi(w.substr(14,2).c_str()) * 60) + atoi(w.substr(17,2).c_str());

                        // cas des doublons
                        if((t2 - t1  == 0) || (dDistance == 0))
                        {
                              TrackSegments[j+1].vitesse = MAXDOUBLE;
                              //MessageDlg("Error computing speed",mtError,TMsgDlgButtons() << mbOK,0);
                        }
                        else
                              TrackSegments[j+1].vitesse =  dDistance / (t2-t1);


                }
                // fixer la vitesse du segment 1 comme celle du segment suivant
                 TrackSegments[0].vitesse =  TrackSegments[1].vitesse;

                // determiner la distance minimum d'une cache vers un track segment
                for(i = 0; i < FoundCachesCounter; i++)
                {
                        CachesFound[i].Distance = MAXDOUBLE;
                        CachesFound[i].Distance2 = MAXDOUBLE;

                        Point1.m_Latitude = CachesFound[i].Latitude;
                        Point1.m_Longitude = CachesFound[i].Longitude;
                        // if (!strcmp(CachesFound[i].GeocacheCode, "GC151JK"))
                        //        Application->MessageBox("Debug","Field Notes Gen",MB_OK);
                        //w = Point1.LatToDMM();
                        //x = Point1.LongToDMM();

                        LabelMessage.sprintf("%04d/%d: Evaluating best distance for %s",i,FoundCachesCounter,CachesFound[i].GeocacheCode);
                        Label2->Caption = LabelMessage;
                        Application->ProcessMessages();


                        for (j = 0; j <  TrackSegmentCounter; j++)
                        {
                                Point2.m_Latitude = TrackSegments[j].Latitude;
                                Point2.m_Longitude = TrackSegments[j].Longitude;

                                // Metric distance evaluation
                                dDistance = Point1.VincentyDistance(Point2, &dFwdAz, &dRevAz);

                                if(Form3->DoubleTrackingEnabled->Checked == False)
                                {
                                  // si on a déja un temps1 de conservé
                                  if (strlen(CachesFound[i].TimeFound) != 0)
                                  {
                                        // evaluate time difference between actual datetime found and datetime from track segment
                                        // transformer UTC en localtime


                                        w = CachesFound[i].TimeFound;
                                        sUTCTime.wYear = atoi(w.substr(0,4).c_str());
                                        sUTCTime.wMonth = atoi(w.substr(5,2).c_str());
                                        sUTCTime.wDay = atoi(w.substr(8,2).c_str());
                                        sUTCTime.wDayOfWeek = 0;
                                        sUTCTime.wHour = atoi(w.substr(11,2).c_str());
                                        sUTCTime.wMinute = atoi(w.substr(14,2).c_str());
                                        sUTCTime.wSecond = atoi(w.substr(17,2).c_str());
                                        sUTCTime.wMilliseconds = 0;
                                        // fonction suivante TROP SLOW!
                                        //SystemTimeToTzSpecificLocalTime(NULL,&sUTCTime, &sLocalTime);
                                        //dt1 = SystemTimeToDateTime(sLocalTime);
                                        dt1 = SystemTimeToDateTime(sUTCTime);


                                        w = TrackSegments[j].TimeSegment;
                                        sUTCTime.wYear = atoi(w.substr(0,4).c_str());
                                        sUTCTime.wMonth = atoi(w.substr(5,2).c_str());
                                        sUTCTime.wDay = atoi(w.substr(8,2).c_str());
                                        sUTCTime.wDayOfWeek = 0;
                                        sUTCTime.wHour = atoi(w.substr(11,2).c_str());
                                        sUTCTime.wMinute = atoi(w.substr(14,2).c_str());
                                        sUTCTime.wSecond = atoi(w.substr(17,2).c_str());
                                        sUTCTime.wMilliseconds = 0;
                                        // fonction suivante TROP SLOW!
                                        //SystemTimeToTzSpecificLocalTime(NULL,&sUTCTime, &sLocalTime);
                                        //dt2 = SystemTimeToDateTime(sLocalTime);
                                        dt2 = SystemTimeToDateTime(sUTCTime);


                                        tdiff = fabs(dt2 - dt1);  // differentiel entre les deux timing

                                        // est-ce que c'est la seconde fois qu'on passe ici apres
                                        // un temps plus grand que le differentiel specifie par DoubleTrackTimeRatio
                                        // et que la distance du track segement a la cache est plus petite que la distance minimale de tracking?
                                        if (tdiff > DoubleTrackTimeRatio)
                                        {
                                            // on est en dehors du temps normal de passage a une cache et on passe encore proche de la cache
                                            if(dDistance <= MinDoubleTrackDist)
                                            {
                                                // est-ce que la distance2 est plus petite que celle d'avant?
                                                if(dDistance < CachesFound[i].Distance2)
                                                {
                                                        // si oui, on update la distance et le datetime trouve pour le second tracking
                                                        CachesFound[i].Distance2 = dDistance;
                                                        CachesFound[i].trkseg2 = j;
                                                        strcpy(CachesFound[i].TimeFound2,TrackSegments[j].TimeSegment);

                                                }
                                             }
                                             else   // est-ce que la distance1 est plus petite que la distance actuelle?
                                             {
                                                if (dDistance < CachesFound[i].Distance)
                                                {
                                                        // si oui, on update la distance et le datetime initialement trouve
                                                        CachesFound[i].Distance = dDistance;
                                                        CachesFound[i].trkseg = j;
                                                        strcpy(CachesFound[i].TimeFound,TrackSegments[j].TimeSegment);
                                                }
                                             }
                                         }
                                         else  // est-ce qu'on est encore en train de passer a cote de la cache.
                                               // C'est donc le cas normal quand on est en train de trouver une cache
                                         {
                                                // est-ce que la distance est plus petite que la distance actuelle?
                                                if (dDistance < CachesFound[i].Distance)
                                                {
                                                        // si oui, on update la distance et le datetime initialement trouve
                                                        CachesFound[i].Distance = dDistance;
                                                        CachesFound[i].trkseg = j;
                                                        strcpy(CachesFound[i].TimeFound,TrackSegments[j].TimeSegment);
                                                }
                                         }

                                  }
                                  else   // la premiere fois on garde tout!
                                  {
                                        CachesFound[i].Distance = dDistance;
                                        CachesFound[i].trkseg = j;
                                        strcpy(CachesFound[i].TimeFound,TrackSegments[j].TimeSegment);

                                  }
                                }
                                else  // comme avant quand on check pas le double tracking
                                {
                                  if (dDistance < CachesFound[i].Distance)
                                  {
                                        CachesFound[i].Distance = dDistance;
                                        CachesFound[i].trkseg = j;
                                        strcpy(CachesFound[i].TimeFound,TrackSegments[j].TimeSegment);
                                  }
                                }
                        }
                }
                // si la distance2 est plus petite que la distance 1, on va interchanger les valeurs
                if (Form3->AutoXchanger->Checked)
                {
                  for(i = 0; i < FoundCachesCounter; i++)
                  {

                   if ((CachesFound[i].Distance2 <  CachesFound[i].Distance) && (TrackSegments[CachesFound[i].trkseg2].vitesse < TrackSegments[CachesFound[i].trkseg].vitesse))
                   {
                     t1 =  CachesFound[i].trkseg2;
                     CachesFound[i].trkseg2 =  CachesFound[i].trkseg;
                     CachesFound[i].trkseg = t1;

                     dt1 = CachesFound[i].Distance2;
                     CachesFound[i].Distance2 = CachesFound[i].Distance;
                     CachesFound[i].Distance = dt1;

                     strcpy(TimeFoundXchange,CachesFound[i].TimeFound2);
                     strcpy(CachesFound[i].TimeFound2,CachesFound[i].TimeFound);
                     strcpy(CachesFound[i].TimeFound,TimeFoundXchange);
                   }

                 }
                }


                // Trier les caches en ordre de decouverte selon TimeFound1
                qsort((void *)CachesFound, FoundCachesCounter, sizeof(t_cachefound), sort_function);

                Screen->Cursor = Save_Cursor; // always restore the cursor

                // pour pouvoir le mettre en clipboard quand on va aller aux field notes page sur gc.com
                strcpy(CurrentOutFileName,OutputNotes);
                OutFileName->Caption = "Field Notes file: ";
                OutFileName->Caption = OutFileName->Caption + OutputNotes;

                // generer le Field Notes file pour uploader eventuellement chez GC.COM

                bufTime[0] = bufCounter[0] = bufPartialCounter[0] = '\0';

                AnsiToUnicode(CurrentOutFileName,&pszOutStringW);
                fpNotes = _wfopen(pszOutStringW,L"wb");
                CoTaskMemFree(pszOutStringW);
                if(fpNotes != NULL)
                {
                        // Special UNICODE value stored at beginning of file
                        // Full Explanation at http://codesnipers.com/?q=node/68
                        // For Unicode files, the BOM ("Byte Order Mark" also called the signature or preamble) is a set of 2 or so bytes
                        // at the beginning used to indicate the type of Unicode encoding.
                        // FF FE:  UCS-2LE or UTF-16LE
                        fwrite("\xff\xfe",1,2,fpNotes);

                        if (Form3->TotalCacheCounterDisplayEnabled->Checked)
                                CacheCounter = atoi(Form3->TotalCacheCounterInitialValue->Text.c_str());

                        FieldNotesCounter = 0;
                        for(i = 0, k=0; i < FoundCachesCounter; i++)
                        {
                             if (CachesFound[i].Distance > TOOFAR_DISTANCE)
                                TooFarCachesCtr++;

                                // Date Time is expressed upon ISO 8601
                                // http://www.w3.org/TR/NOTE-datetime
                                // Times are expressed in UTC (Coordinated Universal Time), with a special UTC designator ("Z").
                                // So 2008-11-08T19:31:32Z is the same as 2008-11-08 14:31:32 in US Eastern Time

                                // transformer UTC en localtime
                                w = CachesFound[i].TimeFound;
                                sUTCTime.wYear = atoi(w.substr(0,4).c_str());
                                sUTCTime.wMonth = atoi(w.substr(5,2).c_str());
                                sUTCTime.wDay = atoi(w.substr(8,2).c_str());
                                sUTCTime.wDayOfWeek = 0;
                                sUTCTime.wHour = atoi(w.substr(11,2).c_str());
                                sUTCTime.wMinute = atoi(w.substr(14,2).c_str());
                                sUTCTime.wSecond = atoi(w.substr(17,2).c_str());
                                sUTCTime.wMilliseconds = 0;
                                dt1 = SystemTimeToDateTime(sUTCTime);

                                SystemTimeToTzSpecificLocalTime(NULL,&sUTCTime, &sLocalTime);

                                tf.tm_sec = sLocalTime.wSecond; tf.tm_min = sLocalTime.wMinute; tf.tm_hour = sLocalTime.wHour;
                                tf.tm_mday = sLocalTime.wDay; tf.tm_mon = sLocalTime.wMonth; tf.tm_year = sLocalTime.wYear;
                                tf.tm_wday = sLocalTime.wDayOfWeek;

                                if(Form3->TimeDisplayEnabled->Checked)
                                {
                                        strftime(bufTime, sizeof(bufTime), Form3->TimeFormatSpecifierCombo->Text.c_str(), &tf);
                                }
                                if (Form3->TotalCacheCounterDisplayEnabled->Checked)
                                {
                                        sprintf(bufCounter,Form3->TotalCacheCounterFormatCombo->Text.c_str(),CacheCounter);
                                        if (CachesFound[i].CacheType == 1) // Standard Found Cache
                                                CacheCounter++;
                                }
                                if (Form3->PartialCacheCounterDisplay->Checked)
                                {
                                        sprintf(bufPartialCounter,Form3->PartialCounterFormat->Text.c_str(),k+1,FoundCachesCounter-DNFCounter);
                                        if (CachesFound[i].CacheType == 1) // Standard Found Cache
                                              k++;
                                }
                                if (CachesFound[i].Distance <= TOOFAR_DISTANCE || Form3->OutTolerance->Checked == True)
                                {

                                        if (CachesFound[i].CacheType == 2)  // DNF Cache ??
                                        {
                                           sprintf(ibf,"%s,%s,%s,\"\"\r\n",
                                                CachesFound[i].GeocacheCode,
                                                CachesFound[i].TimeFound,
                                                "Didn't find it");
                                        }  else {
                                           sprintf(ibf,"%s,%s,%s,\"%s%s%s%s%s\"%s",
                                                CachesFound[i].GeocacheCode,
                                                CachesFound[i].TimeFound,
                                                "Found it" ,
                                                bufCounter,
                                                Form3->TotalCacheCounterDisplayEnabled->Checked && (Form3->TimeDisplayEnabled->Checked || Form3->PartialCacheCounterDisplay->Checked)  ? " ": "",
                                                bufPartialCounter,
                                                (Form3->TotalCacheCounterDisplayEnabled->Checked || Form3->TimeDisplayEnabled->Checked)  && Form3->PartialCacheCounterDisplay->Checked  ? " ": "",
                                                bufTime,
                                                "\r\n");
                                                //Form3->PartialCacheCounterDisplay->Checked || Form3->TotalCacheCounterDisplayEnabled->Checked || Form3->TimeDisplayEnabled->Checked? "\r\n": "");
                                        }
                                        AnsiToUnicode(ibf,&pszOutStringW);
                                        fputws(pszOutStringW,fpNotes);
                                        FieldNotesCounter++;
                                        CoTaskMemFree(pszOutStringW);
                                        //fprintf(fpNotes,"%s,%s,Found it,\"%s\"\n",CachesFound[i].GeocacheCode,CachesFound[i].TimeFound,buf);

                                }

                                sprintf(buf,"%03d",i+1);
                                ListItem = ListView1->Items->Add();
                                ListItem->Caption = buf;

                                ListItem->Data = (void *)&CachesFound[i];

                                ListItem->SubItems->Add(CachesFound[i].GeocacheCode);

                                Point1.m_Latitude = CachesFound[i].Latitude;
                                Point1.m_Longitude = CachesFound[i].Longitude;
                                w = Point1.LatToDMM() + " " + Point1.LongToDMM();
                                ListItem->SubItems->Add(w.c_str());

                                strftime(buf, sizeof(buf),"%m/%d/%y %H:%M:%S",&tf);
                                ListItem->SubItems->Add(buf);

                                sprintf(buf,"%02.4lf",CachesFound[i].Distance);
                                ListItem->SubItems->Add(buf);

                                // si on a un double tracking disponible
                                if (CachesFound[i].Distance2 != MAXDOUBLE)
                                {
                                        // est-il en dehors du temps de passage pres d'une cache
                                        // facon de regler le bug sinon faudrait faire deux passes
                                        w = CachesFound[i].TimeFound2;
                                        sUTCTime.wYear = atoi(w.substr(0,4).c_str());
                                        sUTCTime.wMonth = atoi(w.substr(5,2).c_str());
                                        sUTCTime.wDay = atoi(w.substr(8,2).c_str());
                                        sUTCTime.wDayOfWeek = 0;
                                        sUTCTime.wHour = atoi(w.substr(11,2).c_str());
                                        sUTCTime.wMinute = atoi(w.substr(14,2).c_str());
                                        sUTCTime.wSecond = atoi(w.substr(17,2).c_str());
                                        sUTCTime.wMilliseconds = 0;
                                        dt2 = SystemTimeToDateTime(sUTCTime);
                                        tdiff = fabs(dt2 - dt1);  // differentiel entre les deux timing

                                        if (tdiff > DoubleTrackTimeRatio)
                                        {
                                                SystemTimeToTzSpecificLocalTime(NULL,&sUTCTime, &sLocalTime);

                                                tf.tm_sec = sLocalTime.wSecond; tf.tm_min = sLocalTime.wMinute; tf.tm_hour = sLocalTime.wHour;
                                                tf.tm_mday = sLocalTime.wDay; tf.tm_mon = sLocalTime.wMonth; tf.tm_year = sLocalTime.wYear;
                                                strftime(buf, sizeof(buf),"%m/%d/%y %H:%M:%S",&tf);
                                                ListItem->SubItems->Add(buf);

                                                sprintf(buf,"%02.4lf",CachesFound[i].Distance2);
                                                ListItem->SubItems->Add(buf);

                                                // Vitesse 1
                                                sprintf(buf,"%02.4lf",TrackSegments[CachesFound[i].trkseg].vitesse);
                                                ListItem->SubItems->Add(buf);
                                                // Vitesse 2
                                                sprintf(buf,"%02.4lf",TrackSegments[CachesFound[i].trkseg2].vitesse);
                                                ListItem->SubItems->Add(buf);

                                        }
                                        else
                                        {
                                                // reset values #2
                                                CachesFound[i].Distance2 = MAXDOUBLE;
                                                CachesFound[i].TimeFound2[0] = '\0';
                                                ListItem->SubItems->Add("");// Blank TimeFound2 et dist 2
                                                ListItem->SubItems->Add("");

                                                // Vitesse 1
                                                sprintf(buf,"%02.4lf",TrackSegments[CachesFound[i].trkseg].vitesse);
                                                ListItem->SubItems->Add(buf);

                                                ListItem->SubItems->Add(""); // Blank Vitesse 2

                                        }

                                }
                                else
                                {
                                        ListItem->SubItems->Add("");// Blank TimeFound2 et dist 2
                                        ListItem->SubItems->Add("");

                                        // Vitesse 1
                                        sprintf(buf,"%02.4lf",TrackSegments[CachesFound[i].trkseg].vitesse);
                                        ListItem->SubItems->Add(buf);
                                        
                                        ListItem->SubItems->Add(""); // Blank Vitesse 2

                                }
                                switch(CachesFound[i].CacheType)
                                {
                                        case 0: ListItem->SubItems->Add("Any"); break;
                                        case 1: ListItem->SubItems->Add("Found"); break;
                                        case 2: ListItem->SubItems->Add("DNF"); break;

                                }




                        }
                        fclose(fpNotes);

                        if (Form3->TotalCacheCounterDisplayEnabled->Checked && Form3->SaveIncrementalTotalCounter->Checked)
                                Form3->TotalCacheCounterInitialValue->Text = itoa(CacheCounter,bufCounter,10);

                        Label2->Visible = False;

                        if(TooFarCachesCtr)
                        {

                                LabelMessage.sprintf("Alert! %d geocache(s) out of tolerance. Refer at red line(s) above.\n%d Field Notes were written from %d track segments. ",TooFarCachesCtr,FieldNotesCounter,TrackSegmentCounter);
                        }
                        else
                        {
                                if (FieldNotesCounter)
                                        LabelMessage.sprintf("Fully successful!!!\n%d Field Notes were written from %d track segments.",FieldNotesCounter,TrackSegmentCounter);
                                else
                                        LabelMessage.sprintf("Oups!\n%d Field Notes were extracted from %d track segments.",FieldNotesCounter,TrackSegmentCounter);
                        }
                        MessageDlg(LabelMessage,TooFarCachesCtr || !FieldNotesCounter ? mtWarning:mtInformation,TMsgDlgButtons() << mbOK,0);
                        // Label1->Caption = LabelMessage;

                }
                else
                {
                        MessageDlg("Problem generating output file.",mtError,TMsgDlgButtons() << mbOK,0);
                }

dewouar:
        }
        Screen->Cursor = Save_Cursor; // always restore the cursor
        Process->Enabled = True;
        Button1->Enabled = True;
        GMapViewAll->Enabled = True;
}

int sort_function( const void *a, const void *b)
{
        t_cachefound *x, *y;
        x = (t_cachefound *)a;
        y = (t_cachefound *)b;

        return( strcmp(x->TimeFound,y->TimeFound) );
}


int TrkSsort_function( const void *a, const void *b)
{
        t_trkseg *x, *y;

        x = (t_trkseg *)a;
        y = (t_trkseg *)b;

        return( strcmp(x->TimeSegment,y->TimeSegment) );
        /*
        if(x->Latitude != y->Latitude)
        {
                return (x->Latitude > y->Latitude);
        }
        else
        {
                if(x->Longitude != y->Longitude)
                {
                        return (x->Longitude > y->Longitude);
                }
                else
                        return 0;
        } */
}

//---------------------------------------------------------------------------

// attention ceci est un duplicate ... faut aussi faire les alterations dasn la routine principale
int GenerateFieldNotesFile(int FoundCachesCounter, int OutTolerance, TListItems *lis)
{
        TListItem *li;

        int ret = 0;
        LPOLESTR pszOutStringW;
        FILE *fpNotes;
        int i,k;
        char ibf[8192];
        string w;
        struct tm  *ts, tf;
        char       bufTime[80];
        char       bufCounter[80];
        char       bufPartialCounter[80];
        int     CacheCounter = 0;
        SYSTEMTIME sUTCTime, sLocalTime;
        t_cachefound *CF;

        bufTime[0] = bufCounter[0] = bufPartialCounter[0]  = '\0';


        AnsiToUnicode(CurrentOutFileName,&pszOutStringW);
        fpNotes = _wfopen(pszOutStringW,L"wb");
        CoTaskMemFree(pszOutStringW);
        if(fpNotes != NULL)
        {
                // Special UNICODE value stored at beginning of file
                // Full Explanation at http://codesnipers.com/?q=node/68
                // For Unicode files, the BOM ("Byte Order Mark" also called the signature or preamble) is a set of 2 or so bytes
                // at the beginning used to indicate the type of Unicode encoding.
                // FF FE:  UCS-2LE or UTF-16LE
                fwrite("\xff\xfe",1,2,fpNotes);

                if (Form3->TotalCacheCounterDisplayEnabled->Checked)
                       CacheCounter = atoi(Form3->TotalCacheCounterInitialValue->Text.c_str());


                for(i = 0, k=0; i < FoundCachesCounter; i++)
                {
                        li = lis->Item[i];
                        CF = (t_cachefound *)li->Data;

                        // transformer UTC en localtime
                        w = CF->TimeFound;
                        sUTCTime.wYear = atoi(w.substr(0,4).c_str());
                        sUTCTime.wMonth = atoi(w.substr(5,2).c_str());
                        sUTCTime.wDay = atoi(w.substr(8,2).c_str());
                        sUTCTime.wDayOfWeek = 0;
                        sUTCTime.wHour = atoi(w.substr(11,2).c_str());
                        sUTCTime.wMinute = atoi(w.substr(14,2).c_str());
                        sUTCTime.wSecond = atoi(w.substr(17,2).c_str());
                        sUTCTime.wMilliseconds = 0;
                        //SystemTimeToDateTime(sUTCTime);

                        SystemTimeToTzSpecificLocalTime(NULL,&sUTCTime, &sLocalTime);

                        tf.tm_sec = sLocalTime.wSecond; tf.tm_min = sLocalTime.wMinute; tf.tm_hour = sLocalTime.wHour;
                        tf.tm_mday = sLocalTime.wDay; tf.tm_mon = sLocalTime.wMonth; tf.tm_year = sLocalTime.wYear;
                        tf.tm_wday = sLocalTime.wDayOfWeek;

                        if(Form3->TimeDisplayEnabled->Checked)
                                strftime(bufTime, sizeof(bufTime), Form3->TimeFormatSpecifierCombo->Text.c_str(), &tf);

                        if (Form3->TotalCacheCounterDisplayEnabled->Checked)
                        {
                                sprintf(bufCounter,Form3->TotalCacheCounterFormatCombo->Text.c_str(),CacheCounter);
                                if (CF->CacheType == 1)  // Standard Found Cache
                                        CacheCounter++;
                        }

                        if (Form3->PartialCacheCounterDisplay->Checked)
                        {
                                 sprintf(bufPartialCounter,Form3->PartialCounterFormat->Text.c_str(),k+1,FoundCachesCounter-DNFCounter);
                                 if (CF->CacheType == 1)    // Standard Found Cache
                                              k++;

                        }
                        if (CF->Distance <= TOOFAR_DISTANCE || OutTolerance == True)
                        {
                               if (CF->CacheType == 2)   // DNF ??
                               {
                                        sprintf(ibf,"%s,%s,%s,\"\"\r\n",
                                               CachesFound[i].GeocacheCode,
                                                CachesFound[i].TimeFound,
                                                "Didn't find it");
                               } else
                               {

                                sprintf(ibf,"%s,%s,%s,\"%s%s%s%s%s\"%s",
                                        CF->GeocacheCode,
                                        CF->TimeFound,
                                        "Found it" ,
                                        bufCounter,
                                        Form3->TotalCacheCounterDisplayEnabled->Checked && (Form3->TimeDisplayEnabled->Checked || Form3->PartialCacheCounterDisplay->Checked)  ? " ": "",
                                        bufPartialCounter,
                                        (Form3->TotalCacheCounterDisplayEnabled->Checked || Form3->TimeDisplayEnabled->Checked)  && Form3->PartialCacheCounterDisplay->Checked  ? " ": "",
                                        bufTime,
                                        "\r\n");
                                        //Form3->PartialCacheCounterDisplay->Checked || Form3->TotalCacheCounterDisplayEnabled->Checked || Form3->TimeDisplayEnabled->Checked? "\r\n": "");
                                        

                                }
                                AnsiToUnicode(ibf,&pszOutStringW);
                                fputws(pszOutStringW,fpNotes);
                                CoTaskMemFree(pszOutStringW);


                        }

                 }
                 fclose(fpNotes);
                 if (Form3->TotalCacheCounterDisplayEnabled->Checked && Form3->SaveIncrementalTotalCounter->Checked)
                         Form3->TotalCacheCounterInitialValue->Text = itoa(CacheCounter,bufCounter,10);


                 ret = 1;
        }

        return ret;
}


void __fastcall TForm1::ViewinGSAK1Click(TObject *Sender)
{
        if (ListView1->Selected)
        {
                char ibf[250];
                //sprintf(ibf,"gsak://%%FF/search/%s/Default",ListView1->Selected->SubItems->Strings[0].c_str());
                sprintf(ibf,"gsak://%%FF/search/%s",ListView1->Selected->SubItems->Strings[0].c_str());
                //Application->MessageBox(ibf,"Field Notes Gen",MB_OK);
                ShellExecute(NULL,"open",ibf,NULL,NULL,SW_SHOW);
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ViewatGCCOM1Click(TObject *Sender)
{

        if (ListView1->Selected)
        {
                char ibf[250];

                sprintf(ibf,"http://www.geocaching.com/seek/cache_details.aspx?wp=%s",ListView1->Selected->SubItems->Strings[0].c_str());
                //Application->MessageBox(ibf,"Field Notes Gen",MB_OK);
                ShellExecute(NULL,"open",ibf,NULL,NULL,SW_SHOW);
        }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::LogCacheonline1Click(TObject *Sender)
{
        // Ne fonctionne pas avec un GC Code
        if (ListView1->Selected)
        {
                char ibf[250];

                sprintf(ibf,"http://www.geocaching.com/seek/log.aspx?ID=%s",ListView1->Selected->SubItems->Strings[0].c_str());
                //Application->MessageBox(ibf,"Field Notes Gen",MB_OK);
                ShellExecute(NULL,"open",ibf,NULL,NULL,SW_SHOW);
        }

}
//---------------------------------------------------------------------------


int EditFile(char *s)
{
     char ibf[2048];
     int result;

     sprintf(ibf,"\"%s\"",s);
     result = spawnl(P_NOWAITO,"C:\\Program Files\\Windows NT\\Accessories\\wordpad.exe","wordpad.exe",ibf,NULL);
     if (result == -1)
             Application->MessageBox("Problem launchin editor.","GMapLink",MB_OK);

     return result;
}

void __fastcall TForm1::Button3Click(TObject *Sender)
{
        EditFile(GPXFile->Text.c_str());
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
     char ibf[2048];
     int result;

     sprintf(ibf,"\"%s\"",GPXFile->Text.c_str());
     result = spawnl(P_NOWAITO,"C:\\Garmin\\MapSource.exe","mapsource.exe",ibf,NULL);
     if (result == -1)
             Application->MessageBox("Problem launchin editor.","GMapLink",MB_OK);


}
//---------------------------------------------------------------------------

void __fastcall TForm1::SpeedButton1Click(TObject *Sender)
{
        Clipboard()->SetTextBuf(CurrentOutFileName);
        // ShellExecute(NULL,"open","http://www.geocaching.com/my/fieldnotes.aspx",NULL,NULL,SW_SHOW);
        ShellExecute(NULL,"open","http://www.geocaching.com/my/uploadfieldnotes.aspx",NULL,NULL,SW_SHOW);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::ViewOffline1Click(TObject *Sender)
{
        if (ListView1->Selected)
        {
                char ibf[250];
                sprintf(ibf,"gsak://%%FF/offline/%s/Default",ListView1->Selected->SubItems->Strings[0].c_str());
                //Application->MessageBox(ibf,"Field Notes Gen",MB_OK);
                //Clipboard()->SetTextBuf(ibf);
                ShellExecute(NULL,"open",ibf,NULL,NULL,SW_SHOW);
        }


}
//---------------------------------------------------------------------------

// gpsbabel.exe -p "" -w -t -i gdb -f "C:\Garmin\Geocache\temp\Untitled.gdb" -o gpx -F "C:\Garmin\Geocache\temp\test.gpx"

// Command line to read GPS directly
// gpsbabel.exe -t -w -i garmin -f usb: -o gpx -F blah.gpx
// Utiliser CreateProcess()  ZeroMemory()
void __fastcall TForm1::BabelCallingClick(TObject *Sender)
{
  char ibf[8192];
  int result;
  ErrorCallingBabel = 0;

  if (access(Form3->GPSBabelPath->Text.c_str(),0) != 0)
  {
        MessageDlg("Cannot access GPS Babel. Please correct GPS Babel path in the parameters screen.",mtError,TMsgDlgButtons() << mbOK,0);
        return;
  }

  SaveDialog1->Filter = "GPX files (*.gpx)|*.gpx|All files (*.*)|*.*";
  SaveDialog1->FilterIndex = 1; // start the dialog showing all files
  SaveDialog1->FileName = GPXFile->Text;
  if (SaveDialog1->Execute())
  {
      GPXFile->Clear();
      GPXFile->SelText = SaveDialog1->Files->Strings[0].c_str();

      TCursor Save_Cursor = Screen->Cursor;
      Screen->Cursor = crHourGlass;    // Show hourglass cursor

      sprintf(ibf,"\"%s\" -t -w -i garmin -f usb: -o gpx -F \"%s\"",Form3->GPSBabelPath->Text.c_str(),GPXFile->Text.c_str());
      //sprintf(ibf,"-t -w -i garmin -f usb: -o gpx -F \"%s\"",GPXFile->Text.c_str());
// exemple de ligne d'appel:
// nb: Mars 2011 donne un exception error depuis que j'ai ajouté une uSD card dans mon 62s
// ca n'a rien fait de mieux d'essayer de préciser le USB port ...

// "C:\\Program Files\\GSAK\\gpsbabel.exe" -t -w -i garmin -f usb:7 -o gpx -F "C:\Garmin\Geocache\Geocaches Found\Laurentides\RefugeLacAppelMars2011\Test.gpx"

      try {
        Progress->Show();

        result = CreateProcessEx (
                 //"C:\\Garmin\\Geocache\\C++\\AlphaVal\\AlphaVal.exe", // pour testing
                 Form3->GPSBabelPath->Text.c_str(),
                 ibf,
                 True,
                 False,
                 NULL );

        // result = CreateProcessEx ( Form3->GPSBabelPath->Text.c_str(), ibf,True, False,True, False, NULL );
        Progress->Hide();
        //result = spawnl(P_WAIT,Form3->GPSBabelPath->Text.c_str(),"gpsbabel.exe",ibf,NULL);
        if (result != 0)
        {
             Application->MessageBox("Error calling GPSBabel.","GMapLink",MB_OK);
             ErrorCallingBabel = 1;
        }
      }
      catch (...)
      {
        Application->MessageBox("Exception Error calling GPSBabel.","GMapLink",MB_OK);
        ErrorCallingBabel = 1;
      }
      Screen->Cursor = Save_Cursor; // always restore the cursor

  }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button5Click(TObject *Sender)
{
        BabelCallingClick(NULL);
        if (!ErrorCallingBabel)
                ProcessClick(NULL);
}
//---------------------------------------------------------------------------


void __fastcall TForm1::ListView1CustomDrawItem(TCustomListView *Sender,
      TListItem *Item, TCustomDrawState State, bool &DefaultDraw)
{
       double a = atof(Item->SubItems->Strings[3].c_str());
       if (a > (double)TOOFAR_DISTANCE )
       {
                ListView1->Canvas->Brush->Color = clMaroon;
                ListView1->Canvas->Font->Color = clWhite;
       }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ListView1ColumnClick(TObject *Sender,
      TListColumn *Column)
{
        ColumnToSort = Column->Index;
        ((TCustomListView *)Sender)->AlphaSort();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ListView1Compare(TObject *Sender, TListItem *Item1,
      TListItem *Item2, int Data, int &Compare)
{

 double a, b;

 if (ColumnToSort == 0)
    Compare = CompareText(Item1->Caption,Item2->Caption);
  else
  {
    if (ColumnToSort <= Item1->SubItems->Count && ColumnToSort <= Item2->SubItems->Count)
    {
        int ix = ColumnToSort -1;
        if(ix ==3 || ix==5)   // numeral distance sorting
        {
                a = atof(Item1->SubItems->Strings[ix].c_str());
                b = atof(Item2->SubItems->Strings[ix].c_str());
                Compare = a> b;
        }
        else
                Compare =  CompareText(Item1->SubItems->Strings[ix], Item2->SubItems->Strings[ix]);
    }
    else
        Compare = 0;
  }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button7Click(TObject *Sender)
{
        if (strlen(CurrentOutFileName) > 0)
                ShellExecute(NULL,"open",CurrentOutFileName,NULL,NULL,SW_SHOW);
}
//---------------------------------------------------------------------------

// adapted from from http://www.codeproject.com/KB/threads/CreateProcessEx.aspx
/*The CreateProcessEx function takes four parameters but only the first two are required:

    * lpCmdLine: Pointer to a null-terminated string that specifies the command line to execute. This parameter can be NULL.
    * bWaitForProcess: Indicates if the calling process must wait until child process exits.
    * bMinimizeOnWait: Indicates if the main window will be minimized while the child process runs and automatically restored when it exits.
    * hMainWnd: Handle of the main window to be minimized. If hMainWnd is NULL, then AfxGetMainWnd() is used.
*/
DWORD CreateProcessEx ( LPCSTR lpAppPath, LPCSTR lpCmdLine, BOOL bWaitForProcess, BOOL bMinimizeOnWait, HWND hMainWnd )
{

    STARTUPINFO startupInfo;
    PROCESS_INFORMATION    processInformation;
    char szCmdLine    [ _MAX_PATH  ];

    ZeroMemory( &startupInfo, sizeof( STARTUPINFO ));

    startupInfo.cb = sizeof( STARTUPINFO );

    ZeroMemory( &processInformation, sizeof( PROCESS_INFORMATION ));

    DWORD dwExitCode = -1;
    DWORD WAITCode = 0;
    strcpy( szCmdLine, lpCmdLine );

    if ( CreateProcess(lpAppPath ,    // lpszImageName

                        szCmdLine,                            // lpszCommandLine

                        0,                                    // lpsaProcess

                        0,                                    // lpsaThread

                        FALSE,                                // fInheritHandles

                        DETACHED_PROCESS,                    // fdwCreate
                        //CREATE_NEW_CONSOLE,

                        0,                                    // lpvEnvironment

                        0,                                    // lpszCurDir

                        &startupInfo,                        // lpsiStartupInfo

                        &processInformation                    // lppiProcInfo

                    )) {

        if ( bWaitForProcess ) {

            if ( bMinimizeOnWait )

                if ( IsWindow( hMainWnd )) ShowWindow( hMainWnd, SW_MINIMIZE );
                #ifdef __AFX_H__
                else AfxGetMainWnd()->ShowWindow( SW_MINIMIZE );
                #endif

            WAITCode = WaitForSingleObject( processInformation.hProcess, 5 );
            while ( WAITCode == WAIT_TIMEOUT)
            {
                WAITCode = WaitForSingleObject( processInformation.hProcess, 5 );
                Application->ProcessMessages();
            }

            if ( bMinimizeOnWait )

                if ( IsWindow( hMainWnd )) ShowWindow( hMainWnd, SW_RESTORE );
                #ifdef __AFX_H__
                else AfxGetMainWnd()->ShowWindow( SW_RESTORE );
                #endif

            GetExitCodeProcess( processInformation.hProcess, &dwExitCode );
        }
        else {

            CloseHandle( processInformation.hThread );
            CloseHandle( processInformation.hProcess );

            dwExitCode = 0;
        }
    }

    return dwExitCode;
}
void __fastcall TForm1::Button8Click(TObject *Sender)
{
        if (ListView1->Items->Count)
        {
             if(GenerateFieldNotesFile(ListView1->Items->Count, Form3->OutTolerance->Checked, ListView1->Items))
                 MessageDlg("Done!",mtInformation,TMsgDlgButtons() << mbOK,0);
             else
                 MessageDlg("Error writing output file!",mtError,TMsgDlgButtons() << mbOK,0);
        }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button9Click(TObject *Sender)
{
        int i;
        char buf[100];
        TListItem *li;
        TListItems *lis;
        for (i = 0; i < ListView1->Items->Count; i++)
        {
              sprintf(buf,"%03d",i+1);
              lis = ListView1->Items;
              li = lis->Item[i];
              li->Caption = buf;

        }
        //MessageDlg("Not implemented yet",mtInformation,TMsgDlgButtons() << mbOK,0);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button6Click(TObject *Sender)
{
        AnsiString s;
        double dt1;
        t_cachefound *CF;
        char TimeFoundXchange[25];
        long u;
        TListItems *lis;
        TListItem *li;

        if (ListView1->Selected)
        {
             CF = (t_cachefound *)ListView1->Selected->Data;

             if(CF->Distance2 != MAXDOUBLE)
             {

                li = ListView1->Selected;

                s = ListView1->Selected->SubItems->Strings[4];
                ListView1->Selected->SubItems->Strings[4] = ListView1->Selected->SubItems->Strings[2];
                ListView1->Selected->SubItems->Strings[2] = s;

                  s = ListView1->Selected->SubItems->Strings[5];
                ListView1->Selected->SubItems->Strings[5] = ListView1->Selected->SubItems->Strings[3];
                ListView1->Selected->SubItems->Strings[3] = s;

                s = ListView1->Selected->SubItems->Strings[7];
                ListView1->Selected->SubItems->Strings[7] = ListView1->Selected->SubItems->Strings[6];
                ListView1->Selected->SubItems->Strings[6] = s;

                ListView1->Repaint();



                dt1 = CF->Distance2;
                CF->Distance2 = CF->Distance;
                CF->Distance = dt1;

                strcpy(TimeFoundXchange,CF->TimeFound2);
                strcpy(CF->TimeFound2,CF->TimeFound);
                strcpy(CF->TimeFound,TimeFoundXchange);

                u = CF->trkseg2;
                CF->trkseg2 = CF->trkseg;
                CF->trkseg = u;

                ColumnToSort = 3;
                ListView1->AlphaSort();

                Button9Click(NULL);

                li->Selected = True;
                li->Focused = True;
            }
        }
        //MessageDlg("Not implemented yet",mtInformation,TMsgDlgButtons() << mbOK,0);
}
//---------------------------------------------------------------------------

// from http://support.microsoft.com/kb/138813
HRESULT __fastcall AnsiToUnicode(LPCSTR pszA, LPOLESTR* ppszW)
{

    ULONG cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (NULL == pszA)
    {
        *ppszW = NULL;
        return NOERROR;
    }

    // Determine number of wide characters to be allocated for the
    // Unicode string.
    cCharacters =  strlen(pszA)+1;

    // Use of the OLE allocator is required if the resultant Unicode
    // string will be passed to another COM component and if that
    // component will free it. Otherwise you can use your own allocator.
    *ppszW = (LPOLESTR) CoTaskMemAlloc(cCharacters*2);
    if (NULL == *ppszW)
        return E_OUTOFMEMORY;

    // Covert to Unicode.
    if (0 == MultiByteToWideChar(CP_ACP, 0, pszA, cCharacters,*ppszW, cCharacters))
    {
        dwError = GetLastError();
        CoTaskMemFree(*ppszW);
        *ppszW = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }

    return NOERROR;
}

HRESULT __fastcall UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA)
{

    ULONG cbAnsi, cCharacters;
    DWORD dwError;

    // If input is null then just return the same.
    if (pszW == NULL)
    {
        *ppszA = NULL;
        return NOERROR;
    }

    cCharacters = wcslen(pszW)+1;
    // Determine number of bytes to be allocated for ANSI string. An
    // ANSI string can have at most 2 bytes per character (for Double
    // Byte Character Strings.)
    cbAnsi = cCharacters*2;

    // Use of the OLE allocator is not required because the resultant
    // ANSI  string will never be passed to another COM component. You
    // can use your own allocator.
    *ppszA = (LPSTR) CoTaskMemAlloc(cbAnsi);
    if (NULL == *ppszA)
        return E_OUTOFMEMORY;

    // Convert to ANSI.
    if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, *ppszA,cbAnsi, NULL, NULL))
    {
        dwError = GetLastError();
        CoTaskMemFree(*ppszA);
        *ppszA = NULL;
        return HRESULT_FROM_WIN32(dwError);
    }
    return NOERROR;

}

void __fastcall TForm1::GMapViewAllClick(TObject *Sender)
{
        extern char HeaderGMAP[];
        extern char  GMAP_DATASECTION[];
        extern char ENDGMAP[];
        extern char Gmap1[];
        extern char Gmap2EndHTML[];
        extern char TmpMap[];

        #define NBRCOLORS 6
        char *HTMLColorCodes [NBRCOLORS] = { "#ff0000", "#333399", "#336600","#CC6600", "#FF33FF", "#6600ff" };
        FILE *fp;
        double dDistance;
        double dFwdAz, dRevAz;
        CLatLon Point1, Point2;
        double MoyLat, MoyLong;
        t_cachefound *CF;
        TListItem *li;
        TListItems *lis;
        int i, j;
        int trkdeb, trkfin;
        int SegCounter;
        double MinDist = atof(Form3->MinSegDist->Text.c_str());

        if (ListView1->Items->Count > 0)
        {
                fp = fopen("gmap.html","wt");
                if (fp)
                {
                        MoyLat = MoyLong = 0.0;
                        lis = ListView1->Items;
                        for (i = 0; i < ListView1->Items->Count; i++)
                        {

                                li = lis->Item[i];
                                CF = (t_cachefound *)li->Data;
                                MoyLat += CF->Latitude;
                                MoyLong += CF->Longitude;
                        }
                        MoyLat = MoyLat / ListView1->Items->Count;
                        MoyLong = MoyLong / ListView1->Items->Count;

                        fprintf(fp,Gmap1,MoyLat,MoyLong);


                        // a reprendre quand on aura l'icone d'une geocache coffre
                        fprintf(fp,"\n       var cacheicon32 = new GIcon();\n"
                                   "       cacheicon32.image = 'http://opusterra.gel.usherbrooke.ca/Maps/Img/found-32.png';\n"
                                   "       cacheicon32.iconSize = new GSize(32, 32);\n"
                                   "       cacheicon32.iconAnchor = new GPoint(16, 16);\n"
                                   "       cacheicon32.infoWindowAnchor = new GPoint(25, 7);\n\n");

                        fprintf(fp,"\n       var cacheicon16 = new GIcon();\n"
                                   "       cacheicon16.image = 'http://opusterra.gel.usherbrooke.ca/Maps/Img/found-16.png';\n"
                                   "       cacheicon16.iconSize = new GSize(16, 16);\n"
                                   "       cacheicon16.iconAnchor = new GPoint(16, 16);\n"
                                   "       cacheicon16.infoWindowAnchor = new GPoint(25, 7);\n\n");

                        fprintf(fp,"\n       var cacheiconDNF = new GIcon();\n"
                                   "       cacheiconDNF.image = 'http://opusterra.gel.usherbrooke.ca/Maps/Img/DNF.png';\n"
                                   "       cacheiconDNF.iconSize = new GSize(15, 15);\n"
                                   "       cacheiconDNF.iconAnchor = new GPoint(15, 15);\n"
                                   "       cacheiconDNF.infoWindowAnchor = new GPoint(25, 7);\n\n");

                        for (i = 0; i < ListView1->Items->Count; i++)
                        {

                                li = lis->Item[i];
                                CF = (t_cachefound *)li->Data;
                                MoyLat += CF->Latitude;
                                MoyLong += CF->Longitude;


                                fprintf(fp,"        var latlng = new GLatLng(%lf,%lf);\n"
                                           "        var marker%i = new GMarker(latlng, {icon: %s, title: '%s-%s'});\n"
                                           //"        var marker%i = new GMarker(latlng, {title: '%s-%s'} );\n"
                                           "        GEvent.addListener(marker%i, 'click', function() {\n"
                                           "             var myHtml = \"<b>%s-%s</b><br><br> %s at %s<br>with a distance of %.2lf meters from the nearest trace. <br><BR> <a href='gsak://%%FF/search/%s'>View in GSAK</a><BR><BR> \";\n"
                                           "            marker%i.openInfoWindowHtml(myHtml);});  \n"
                                           "        map.addOverlay(marker%i);\n\n",
                                                CF->Latitude, CF->Longitude,
                                                i,
                                                CF->CacheType == 2 ? "cacheiconDNF" : "cacheicon16",
                                                li->Caption.c_str(), CF->GeocacheCode,
                                                i,
                                                li->Caption.c_str(),CF->GeocacheCode,
                                                CF->CacheType == 2 ? "DNF" : "Found",
                                                li->SubItems->Strings[2].c_str(),CF->Distance,CF->GeocacheCode,
                                                i,
                                                i);
                        }

                        SegCounter = 0;
                        
                        {

                           fprintf(fp,         "        var polyline%d = new GPolyline([\n",SegCounter);

                           // initialisation du premier point de segment
                           li = lis->Item[0];
                           CF = (t_cachefound *)li->Data;
                           Point1.m_Latitude = TrackSegments[CF->trkseg].Latitude;
                           Point1.m_Longitude = TrackSegments[CF->trkseg].Longitude;

                           for(i = 0; i < ListView1->Items->Count-1; i++)
                           {
                                li = lis->Item[i];
                                CF = (t_cachefound *)li->Data;
                                trkdeb = CF->trkseg;

                                li = lis->Item[i+1];
                                CF = (t_cachefound *)li->Data;
                                trkfin = CF->trkseg;

                                // verification de direction + ou - pour s'assurer qu'on va d'un point
                                // a un autre meme si on retourne sur nos pas. Cas quand on switch les valeurs du TimeFound
                                if(trkdeb > trkfin)
                               {

                                        for (j = trkdeb; j >= trkfin; j--)
                                        {
                                                Point2.m_Latitude = TrackSegments[j].Latitude;
                                                Point2.m_Longitude = TrackSegments[j].Longitude;
                                                dDistance = Point1.VincentyDistance(Point2, &dFwdAz, &dRevAz);
                                                if ( fabs(dDistance) > MinDist)
                                                {
                                                        fprintf(fp,"\n        ], \"%s\", 4);\n", HTMLColorCodes[SegCounter%NBRCOLORS]);
                                                        fprintf(fp,"        map.addOverlay(polyline%d);\n\n",SegCounter);
                                                        SegCounter++;
                                                        fprintf(fp,         "        var polyline%d = new GPolyline([\n",SegCounter);
                                                }
                                                fprintf(fp,"          new GLatLng(%lf, %lf),\n",TrackSegments[j].Latitude,TrackSegments[j].Longitude);
                                                Point1.m_Latitude = TrackSegments[j].Latitude;
                                                Point1.m_Longitude = TrackSegments[j].Longitude;

                                        }

                                }
                                else
                                {
                                        for (j = trkdeb; j <= trkfin; j++)
                                        {
                                                Point2.m_Latitude = TrackSegments[j].Latitude;
                                                Point2.m_Longitude = TrackSegments[j].Longitude;
                                                dDistance = Point1.VincentyDistance(Point2, &dFwdAz, &dRevAz);
                                                if ( fabs(dDistance) > MinDist)
                                                {
                                                        fprintf(fp,"\n        ], \"%s\", 4);\n", HTMLColorCodes[SegCounter%NBRCOLORS]);
                                                        fprintf(fp,"        map.addOverlay(polyline%d);\n\n",SegCounter);
                                                        SegCounter++;
                                                        fprintf(fp,         "        var polyline%d = new GPolyline([\n",SegCounter);
                                                }

                                                fprintf(fp,"          new GLatLng(%lf, %lf),\n",TrackSegments[j].Latitude,TrackSegments[j].Longitude);
                                                Point1.m_Latitude = TrackSegments[j].Latitude;
                                                Point1.m_Longitude = TrackSegments[j].Longitude;

                                        }
                                }

                            }

                            fprintf(fp,"\n        ], \"%s\", 4);\n", HTMLColorCodes[SegCounter%NBRCOLORS]);
                            fprintf(fp,"        map.addOverlay(polyline%d);\n",SegCounter);
                            //SegCounter++;
                            //AutreSegment = 0;
                        }

                        fprintf(fp, Gmap2EndHTML);


                        fclose(fp);


                        ShellExecute(NULL,"open","gmap.html",NULL,NULL,SW_SHOW);

                }
        }

}
//---------------------------------------------------------------------------

char Gmap1[] =

"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
"  \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
"  <head>\n"
"    <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>\n"
"    <title>Google Maps Export from Field Notes Generator</title>\n"
"    <script src=\"http://maps.google.com/maps?file=api&amp;v=2&amp;key=ABQIAAAALiU1eKD8_HPgDMtdy2uB3BRuO8epP0MGRgEniQNkIlYhRY1E7BRWVBlxj5IpuozkfPuQJ1cQ_sJmQg\"\n"
"      type=\"text/javascript\"></script>\n"
"    <script type=\"text/javascript\">\n"
"\n"
"    //<![CDATA[\n"
"\n"
"    function load() {\n"
"      if (GBrowserIsCompatible()) {\n"
"        var map = new GMap2(document.getElementById(\"map\"));\n"
"        map.setCenter(new GLatLng(%lf, %lf), 13);\n"
"        map.setMapType(G_PHYSICAL_MAP);\n"
"        map.addMapType(G_PHYSICAL_MAP);\n"
"        map.addControl(new GMapTypeControl());\n"
"        map.addControl(new GSmallMapControl());\n"
"        map.enableContinuousZoom();\n"
"        map.enableScrollWheelZoom();\n"
"\n";

char Gmap2EndHTML[] =
"\n"
"\n"
"      }\n"
"    }\n"
"\n"
"    //]]>\n"
"    </script>\n"
"  </head>\n"
"  <body onload=\"load()\" onunload=\"GUnload()\">\n"
"    <div id=\"map\" style=\"width: 800px; height: 600px\"></div>\n"
"  </body>\n"
"</html>\n"
"\n";




                        /*
                        li = lis->Item[0];
                        CF = (t_cachefound *)li->Data;
                        trkdeb = CF->trkseg;

                        li = lis->Item[ListView1->Items->Count-1];
                        CF = (t_cachefound *)li->Data;
                        trkfin = CF->trkseg;

                        if(trkdeb > trkfin)
                        {
                                long u;
                                u = trkfin;
                                trkfin = trkdeb;
                                trkdeb = u;
                        }
                        for (i = trkdeb; i < trkfin; i++)
                        {
                                fprintf(fp,"          new GLatLng(%lf, %lf)",TrackSegments[i].Latitude,TrackSegments[i].Longitude);
                                //if (i < trkfin-1)  // pas necessaire d'omettre la virgule pour le dernier item
                                    fprintf(fp,",\n");
                        }
                        */


void __fastcall TForm1::ViewinGoogleMap1Click(TObject *Sender)
{
  t_cachefound *CF;
  TListItem *li;
  TListItems *lis;
  FILE *fp;

  if (ListView1->Selected)
  {
    CF = (t_cachefound *)ListView1->Selected->Data;
    fp = fopen("gmap.html","wt");
    if (fp)
    {
        fprintf(fp,Gmap1,CF->Latitude,CF->Longitude);

        // a reprendre quand on aura l'icone d'une geocache coffre
        fprintf(fp,"\n       var cacheicon32 = new GIcon();\n"
                                   "       cacheicon32.image = 'http://opusterra.gel.usherbrooke.ca/Maps/Img/found-32.png';\n"
                                   "       cacheicon32.iconSize = new GSize(32, 32);\n"
                                   "       cacheicon32.iconAnchor = new GPoint(16, 16);\n"
                                   "       cacheicon32.infoWindowAnchor = new GPoint(25, 7);\n\n");

        fprintf(fp,"\n       var cacheicon16 = new GIcon();\n"
                                   "       cacheicon16.image = 'http://opusterra.gel.usherbrooke.ca/Maps/Img/found-16.png';\n"
                                   "       cacheicon16.iconSize = new GSize(16, 16);\n"
                                   "       cacheicon16.iconAnchor = new GPoint(16, 16);\n"
                                   "       cacheicon16.infoWindowAnchor = new GPoint(25, 7);\n\n");
        fprintf(fp,"\n       var cacheiconDNF = new GIcon();\n"
                                   "       cacheiconDNF.image = 'http://opusterra.gel.usherbrooke.ca/Maps/Img/DNF.png';\n"
                                   "       cacheiconDNF.iconSize = new GSize(15, 15);\n"
                                   "       cacheiconDNF.iconAnchor = new GPoint(15, 15);\n"
                                   "       cacheiconDNF.infoWindowAnchor = new GPoint(25, 7);\n\n");

        li = ListView1->Selected;

        fprintf(fp,"        var latlng = new GLatLng(%lf,%lf);\n"
                   "        var marker%i = new GMarker(latlng, {icon: %s, title: '%s-%s'});\n"
                 //"        var marker%i = new GMarker(latlng, {title: '%s-%s'} );\n"
                   "        GEvent.addListener(marker%i, 'click', function() {\n"
                   "             var myHtml = \"<b>%s-%s</b><br><br> %s at %s<br>with a distance of %.2lf meters from the nearest trace. <br><BR> <a href='gsak://%%FF/search/%s'>View in GSAK</a><BR><BR> \";\n"
                   "            marker%i.openInfoWindowHtml(myHtml);});  \n"
                   "        map.addOverlay(marker%i);\n\n",
                                  CF->Latitude, CF->Longitude,
                                  1,
                                  CF->CacheType == 2 ? "cacheiconDNF" : "cacheicon16",
                                  li->Caption.c_str(), CF->GeocacheCode,
                                  1,
                                  li->Caption.c_str(),CF->GeocacheCode,
                                  CF->CacheType == 2 ? "DNF" : "Found",
                                  li->SubItems->Strings[2].c_str(),

                                  CF->Distance,CF->GeocacheCode,
                                  1,
                                  1);
         fprintf(fp, Gmap2EndHTML);

         fclose(fp);


         ShellExecute(NULL,"open","gmap.html",NULL,NULL,SW_SHOW);


    }
  }
}

//---------------------------------------------------------------------------

void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
   TIniFile *ini;
   ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ) );
   ini->WriteString ( "General", "GPXFile", GPXFile->Text.c_str()  );
   ini->WriteString ( "General", "Tolerance", Form3->Tolerance->Text.c_str()  );
   ini->WriteBool( "General", "OutTolerance", Form3->OutTolerance->Checked );
   ini->WriteBool( "General", "DoubleTrackingEnabled", Form3->DoubleTrackingEnabled->Checked );
   ini->WriteString ( "General", "DoubleTrackDist", Form3->DoubleTrackDist->Text.c_str()  );
   ini->WriteString ( "General", "DoubleTrackTime", Form3->DoubleTrackTime->Text.c_str()  );
   ini->WriteString( "General", "MinSegDist",Form3->MinSegDist->Text.c_str() );
   ini->WriteBool( "General", "AutoXchanger", Form3->AutoXchanger->Checked );
   ini->WriteInteger("General","GeocachingCodePosition", Form3->GeocachingCodePosition->ItemIndex);
   ini->WriteString ( "General", "GPSBabelExecPath", Form3->GPSBabelPath->Text.c_str());
   ini->WriteBool( "General", "TotalCacheCounterDisplayEnabled",Form3->TotalCacheCounterDisplayEnabled->Checked);
   ini->WriteString("General","TotalCacheCounterInitialValue",Form3->TotalCacheCounterInitialValue->Text.c_str());
   // ini->WriteString("General","TotalCacheCounterFormat",Form3->TotalCacheCounterFormat->Text.c_str());
   ini->WriteString("General","TotalCacheCounterFormatCombo",Form3->TotalCacheCounterFormatCombo->Text.c_str());
   ini->WriteBool( "General", "TimeDisplayEnabled",Form3->TimeDisplayEnabled->Checked);
   //ini->WriteString("General","TimeFormatSpecifier",Form3->TimeFormatSpecifier->Text.c_str());
   ini->WriteString("General","TimeFormatSpecifierCombo",Form3->TimeFormatSpecifierCombo->Text.c_str());
   ini->WriteBool( "General", "SaveIncrementalTotalCounter",Form3->SaveIncrementalTotalCounter->Checked);
   ini->WriteBool( "General", "PartialCacheCounterDisplay",Form3->PartialCacheCounterDisplay->Checked);
   ini->WriteString("General","PartialCounterFormat",Form3->PartialCounterFormat->Text);
   ini->WriteBool( "General", "GCCodeListGenerate",Form3->GCCodeListGenerate->Checked);
   ini->WriteString("General","GCCodeListFilename",Form3->GCCodeListFilename->Text.c_str());
   ini->WriteBool("General","DNFChecking",Form3->DNFChecking->Checked);
   ini->WriteInteger("General","DNFIcon",Form3->DNFIcon->ItemIndex);
   delete ini;
}

//---------------------------------------------------------------------------


void __fastcall TForm1::FormShow(TObject *Sender)
{
   TIniFile *ini;
   GPXFile->Clear();
   Application->HintHidePause = 5000;
   
   ini = new TIniFile(ChangeFileExt( Application->ExeName, ".INI" ) );
   GPXFile->SelText =  ini->ReadString( "General", "GPXFile","" );
   Form3->Tolerance->Text =  ini->ReadString( "General", "Tolerance","20" );
   Form3->OutTolerance->Checked = ini->ReadBool( "General", "OutTolerance","False" );
   Form3->DoubleTrackingEnabled->Checked = ini->ReadBool( "General", "DoubleTrackingEnabled","False" );
   Form3->DoubleTrackDist->Text = ini->ReadString( "General", "DoubleTrackDist","15" );
   Form3->DoubleTrackTime->Text = ini->ReadString( "General", "DoubleTrackTime","50" );
   Form3->MinSegDist->Text = ini->ReadString( "General", "MinSegDist","500" );
   Form3->AutoXchanger->Checked = ini->ReadBool( "General", "AutoXchanger","False" );
   Form3->GPSBabelPath->Text = ini->ReadString( "General", "GPSBabelExecPath","C:\\Program Files\\GSAK\\gpsbabel.exe");
   Form3->GeocachingCodePosition->ItemIndex = ini->ReadInteger("General","GeocachingCodePosition",0);
   Form3->TotalCacheCounterDisplayEnabled->Checked = ini->ReadBool( "General", "TotalCacheCounterDisplayEnabled",1);
   Form3->TotalCacheCounterInitialValue->Text = ini->ReadString("General","TotalCacheCounterInitialValue",0);
   //Form3->TotalCacheCounterFormat->Text = ini->ReadString("General","TotalCacheCounterFormat","#%d");
   Form3->TotalCacheCounterFormatCombo->Text = ini->ReadString("General","TotalCacheCounterFormatCombo","#%d");
   Form3->TimeDisplayEnabled->Checked = ini->ReadBool( "General", "TimeDisplayEnabled",1);
   // Form3->TimeFormatSpecifier->Text = ini->ReadString("General","TimeFormatSpecifier","%Hh%M"); // %I:%M%p
   Form3->TimeFormatSpecifierCombo->Text = ini->ReadString("General","TimeFormatSpecifierCombo","%Hh%M"); // %I:%M%p
   Form3->SaveIncrementalTotalCounter->Checked = ini->ReadBool( "General", "SaveIncrementalTotalCounter",1);
   Form3->PartialCacheCounterDisplay->Checked = ini->ReadBool( "General", "PartialCacheCounterDisplay",0);
   Form3->PartialCounterFormat->Text = ini->ReadString("General","PartialCounterFormat","(%d/%d)");
   Form3->GCCodeListGenerate->Checked = ini->ReadBool( "General", "GCCodeListGenerate",0);
   Form3->GCCodeListFilename->Text = ini->ReadString("General","GCCodeListFilename","c:\\temp\\GCcodelist.lst");
   Form3->DNFChecking->Checked = ini->ReadBool("General","DNFChecking",0);
   Form3->DNFIcon->ItemIndex = ini->ReadInteger("General","DNFIcon",61); // Default to Ghost Town

   OutFileName->Caption = "";

   Button1->SetFocus();

   delete ini;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Options1Click(TObject *Sender)
{
        Form3->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Exit2Click(TObject *Sender)
{
        Close();
}
//---------------------------------------------------------------------------



void __fastcall TForm1::FormCreate(TObject *Sender)
{
        // Screen->Cursors[1] =   LoadCursor(HInstance, "WEBPOINTER");

}
//---------------------------------------------------------------------------

void __fastcall TForm1::Label11Click(TObject *Sender)
{
        ShellExecute(0, "open",  "http://opusterra.gel.usherbrooke.ca/", "", "",  SW_SHOWNORMAL);

}
//---------------------------------------------------------------------------

void __fastcall TForm1::FRancais1Click(TObject *Sender)
{
        ShellExecute(0, "open",  "Lismoi.htm", "", "",  SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::English1Click(TObject *Sender)
{
        Application->MessageBox("To come","Field Notes Gen",MB_OK);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SpeedButton2Click(TObject *Sender)
{
        Clipboard()->SetTextBuf(CurrentOutFileName);
}
//---------------------------------------------------------------------------

