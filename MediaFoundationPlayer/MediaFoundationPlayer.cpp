// MediaFoundationPlayer.cpp : Defines the entry point for the application.
//

#include "StdAfx.h"
#include "MediaFoundationPlayer.h"

#include "CPlayer.h"

#include "mfapi.h"
#include "mfidl.h"

// **************************************************************************
// **************************************************************************
// **************************************************************************
// *********** DEMO VIDEO PLAYER CON MICROSOFT MEDIA FOUNDATION *************

// Ejecutar la aplicación y seleccionar del menú principal "Reproducir Video"

// Uso general:
// En el menú superior elegir la opción "Cargar video"
// Luego iniciar la reproducción con la barra espaciadora
// Utilizar el resto de los controles...
// Cerrar la aplicación al terminar

// Mapa de teclas de control de reproducción

// ESPACIO - Play / Pausa
// Q - Salto hacia atrás 5 segundos
// W - Salto hacia adelante 5 segundos
// E - Volumen de audio máximo 1.0
// R - Volumen de audio 0.5
// T - Volumen de audio nulo 0.0 (mute)
// A - Velocidad de reproducción x1.0 (normal)
// S - Velocidad de reproducción x2.0 (rápido)
// D - Velocidad de reproducción x0.5 (lento)
// Z - Velocidad de reproducción x4.0 (muy rápido)
// X - Deprecated
// ENTER - Cambiar de video

// **************************************************************************
// **************************************************************************
// **************************************************************************

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;							// current instance
TCHAR szTitle[MAX_LOADSTRING];				// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];		// the main window class name
BOOL g_bRepaintClient = TRUE;				// Repaint the application client area?

bool flag_video_actual = true;				// Se utiliza para alternar entre 2 videos posibles

// Global variable to PLAYER
int instanciasCPlayer = 0; // burdo pero eficaz, control de unicidad de instanciacion de pCPlayer
CPlayer* pCPlayer = NULL;
HWND ventana_video;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MEDIAFOUNDATIONPLAYER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MEDIAFOUNDATIONPLAYER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	int retorno = (int) msg.wParam;

	MFShutdown();

	return retorno;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEDIAFOUNDATIONPLAYER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MEDIAFOUNDATIONPLAYER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   if (S_OK != MFStartup(MF_VERSION, MFSTARTUP_FULL))
   {
	   return FALSE;
   }
   
   return TRUE;
}

// ---------------------------------------------------------------------------------------

// Update the application UI to reflect the current state.
void UpdateUI(HWND hwnd, PlayerState state)
{
    BOOL bWaiting = FALSE;
    BOOL bPlayback = FALSE;

    assert(pCPlayer != NULL);

    switch (state)
    {
    case OpenPending:
        bWaiting = TRUE;
        break;

    case Started:
        bPlayback = TRUE;
        break;

    case Paused:
        bPlayback = TRUE;
        break;
    }

    HMENU hMenu = GetMenu(hwnd);
    UINT  uEnable = MF_BYCOMMAND | (bWaiting ? MF_GRAYED : MF_ENABLED);

    // EnableMenuItem(hMenu, ID_FILE_OPENFILE, uEnable);
    // EnableMenuItem(hMenu, ID_FILE_OPENURL, uEnable);

    if (bPlayback && pCPlayer->HasVideo())
    {
        g_bRepaintClient = FALSE;
    }
    else
    {
        g_bRepaintClient = TRUE;
    }
}

//  Handler for WM_PAINT messages.
void OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    if (pCPlayer && pCPlayer->HasVideo())
    {
        // Video is playing. Ask the player to repaint.
        pCPlayer->Repaint();
    }
    else
    {
        // The video is not playing, so we must paint the application window.
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH) COLOR_WINDOW);
    }
    EndPaint(hwnd, &ps);
}

//  Handler for WM_CREATE message.
LRESULT OnCreateWindow(HWND hwnd)
{
    // Initialize the player object.
    HRESULT hr = CPlayer::CreateInstance(hwnd, hwnd, &pCPlayer); 
    if (SUCCEEDED(hr))
    {
        UpdateUI(hwnd, Closed);
        return 0;   // Success.
    }
    else
    {
        // NotifyError(NULL, L"Could not initialize the player object.", hr);
        return -1;  // Destroy the window
    }
}

// Handler for Media Session events.
void OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr)
{
    HRESULT hr = pCPlayer->HandleEvent(pUnkPtr);
    if (FAILED(hr))
    {
		// Error
        // NotifyError(hwnd, L"An error occurred.", hr);
    }
    UpdateUI(hwnd, pCPlayer->GetState());
}

// Handler for WM_CHAR messages. 
void OnKeyPress(WPARAM key)
{
	PlayerState state = pCPlayer->GetState();

	// Códigos numéricos de caracteres: http://www.expandinghead.net/keycode.html

	// *************************
	// Velocidades de reproducción permitidas 
	
	float minimum_rate_forward = pCPlayer->getMinimumSupportedRate(MFRATE_DIRECTION::MFRATE_FORWARD, false);

	float minimum_rate_backward = pCPlayer->getMinimumSupportedRate(MFRATE_DIRECTION::MFRATE_REVERSE, true);

	float closest_allowed_rate = 0.0;
	// *************************

    switch (key)
    {
    // Space key toggles between running and paused
    case VK_SPACE:	// ESPACIO	-	Play / Pausa
        if (state == Started)
        {
            pCPlayer->Pause();

			// ---
			// Código para Debug: control de valores calculados sobre ejecución del video
			// Métodos de cálculo de números de cuadro y tiempo de reproducción. Resultados OK.
			// int fp1000s = pCPlayer->getFrameRateInFramesPer1000Seconds(); // OK
			// int num_cuadro1 = pCPlayer->obtenerNumeroDeFrameSegunTiempo(0.0); // OK
			// int num_cuadro2 = pCPlayer->obtenerNumeroDeFrameSegunTiempo(1.0); // OK
			// int num_cuadro3 = pCPlayer->obtenerNumeroDeFrameSegunTiempo(10.0); // OK
			// int cuadros_totales = pCPlayer->obtenerNumeroDeFramesTotales(); // OK
			// ---
        }
		else if (state == Paused || state == Ready || state == Stopped)
        {
            pCPlayer->Play();
        }
        break;

	case 113: // Q	-	Salto hacia atrás 5 segundos
		if (state == Started || state == Paused || state == Stopped || state == Ready)
		{
			pCPlayer->Pause();
			double time = pCPlayer->getActualPlayingTime();
			double tiempo_con_salto = (time - 5.0) * 10000000;
			if (tiempo_con_salto > 0 )
			{
				pCPlayer->SeekAndPlay(tiempo_con_salto);
			}
			else
			{
				pCPlayer->SeekAndPlay(time * 10000000);
			}
		}
		break;

	case 119: // W	-	Salto hacia adelante 5 segundos
		if (state == Started || state == Paused || state == Stopped || state == Ready)
		{
			pCPlayer->Pause();
			double time = pCPlayer->getActualPlayingTime();
			double tiempo_con_salto = (time + 5.0) * 10000000;
			MFTIME tiempo_total;
			pCPlayer->GetTotalDurationTime(&tiempo_total);
			double tiempo_total_d = (double)tiempo_total;
			double total_menos_saltoActual = tiempo_total_d - tiempo_con_salto;
			if ( total_menos_saltoActual > 0.0 )
			{
				pCPlayer->SeekAndPlay(tiempo_con_salto);
			}
			else
			{
				if (time * 10000000 < tiempo_total)
				{
					pCPlayer->SeekAndPlay(time * 10000000);
				}
			}
		}
		break;

	case 97:	// A	-	Velocidad de reproducción normal
		pCPlayer->SetPlaybackRate(1.0f, false); // Reproducción normal
		break;
		
	case 115:	// S	-	Velocidad de reproducción x2
		pCPlayer->SetPlaybackRate(2.0f, false); // Sin submuestrear se llega bien a 2x
		break;

	case 100:	// D	-	Velocidad de reproducción x0.5
		pCPlayer->SetPlaybackRate(minimum_rate_forward, false); // Sin submuestrear se llega bien a 0.5x
		break;

	case 122:	// Z	-	Velocidad de reproducción x4
		pCPlayer->SetPlaybackRate(4.0f, true); // Se submuestrea el video para que pueda llegar a 4x
		break;
	
	case 120:	// X . Por el momento se desestima reproducir videos hacia atrás
		// bool ok = false;
		// ok = pCPlayer->isPlaybackRateSupported(minimum_rate_backward, true, &closest_allowed_rate);
		// pCPlayer->SetPlaybackRate(minimum_rate_backward, true);
		break;

	case 101:	// E	-	Volumen de audio normal
		pCPlayer->setAudioVolume(1.0);
		break;

	case 114:	// R	-	Volumen de audio bajo
		pCPlayer->setAudioVolume(0.5);
		break;

	case 116:	// T	-	Volumen de audio mudo (mute)
		pCPlayer->setAudioVolume(0.0);
		break;

	case 13:	// ENTER -	Cambiar de video al vuelo o detenido

		if (flag_video_actual)
		{
			pCPlayer->OpenURL(_T("AFX_Polarizados.avi"));
			pCPlayer->Play();
			flag_video_actual = false;
		}
		else
		{
			pCPlayer->OpenURL(_T("test-mpeg_512kb.mp4"));
			pCPlayer->Play();
			flag_video_actual = true;
		}
		break;
    }
}

// ---------------------------------------------------------------------------------------

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//	WM_CREATE					- Process window creation
//  WM_COMMAND					- process the application menu
//  WM_PAINT					- Paint the main window
//  WM_DESTROY					- Post a quit message and return
//	WM_ERASEBKGND				- Suppress window erasing, to reduce flickering while the video is playing.
//	WM_PAINT					- Painting of the window
//	WM_DESTROY					- Window destroy message
//	WM_CHAR						- Process key pressed
//	WM_APP_PLAYER_EVENT			- Process Media Player Event
// 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	// Media Player Events
	IMFMediaEvent* evento_media_player = NULL;
	MediaEventType meType = MEUnknown;  // Event type

	switch (message)
	{
	case WM_CREATE:
		return OnCreateWindow(hWnd);
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FILE_PLAYVIDEO:
					
			pCPlayer->OpenURL(_T("test-mpeg_512kb.mp4"));
			
			// Para debug, comprobación de tamaño de frame original
			int alto, ancho;
			pCPlayer->getFrameSize(alto, ancho);

			// pCPlayer->Play();
			break;

		// case ID_FILE_PLAYVIDEO2:
			// pCPlayer->Play();
			// break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_ERASEBKGND:
        // Suppress window erasing, to reduce flickering while the video is playing.
        return 1;
		break;

	case WM_PAINT:
		OnPaint(hWnd);
		break;

	case WM_DESTROY:
		if(pCPlayer)
		{
			// if (pCPlayer->GetState() == Started)
			// {
				pCPlayer->Shutdown();
				pCPlayer->Release();
			// }
		}
		PostQuitMessage(0);
		break;

	case WM_CHAR:
        OnKeyPress(wParam);
        break;

	case WM_APP_PLAYER_EVENT:
		OnPlayerEvent(hWnd, wParam);
        break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// ---------------------------------------------------------------------------------------

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
