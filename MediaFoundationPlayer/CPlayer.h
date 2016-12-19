// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef PLAYER_H
#define PLAYER_H

#include <new>
#include <windows.h>
#include <shobjidl.h> 
#include <shlwapi.h>
#include <assert.h>
#include <strsafe.h>

#include "MediaFoundationPlayer.h"

#include <Mfobjects.h>
#include "mfapi.h"
#include "mfidl.h"

// Media Foundation headers
#include <mfapi.h>
#include <mfidl.h>
#include <mferror.h>
#include <evr.h>

#include "StdAfx.h"

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

// DEFICION DE MENSAJE DE CPLAYER EN LA APLICACION
// Nota: mover a aplicación esta línea y comentarla en este lugar:
const UINT WM_APP_PLAYER_EVENT = WM_APP + 1;   

// WPARAM = IMFMediaEvent*, WPARAM = MediaEventType

enum PlayerState
{
    Closed = 0,     // No session.
    Ready,          // Session was created, ready to open a file. 
    OpenPending,    // Session is opening a file.
    Started,        // Session is playing a file.
    Paused,         // Session is paused.
    Stopped,        // Session is stopped (ready to play). 
    Closing         // Application has closed the session, but is waiting for MESessionClosed.
};

class CPlayer : public IMFAsyncCallback
{
public:
    static HRESULT CreateInstance(HWND hVideo, HWND hEvent, CPlayer **ppPlayer);

    // IUnknown methods
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMFAsyncCallback methods
    STDMETHODIMP  GetParameters(DWORD*, DWORD*)
    {
        // Implementation of this method is optional.
        return E_NOTIMPL;
    }
    STDMETHODIMP  Invoke(IMFAsyncResult* pAsyncResult);

    // Playback
    HRESULT       OpenURL(const WCHAR *sURL); // Abre archivo de video
    HRESULT       Play();
    HRESULT       Pause();
    HRESULT       Stop();
	HRESULT       SeekAndPlay(int time); // tiempo en unidades de 100 nanosegundos desde el inicio del video
	double		  getActualPlayingTime(); // devuelve tiempo actual de reproducción de video en segundos. Devuelve -1.0 si no hay sesión activa de video.
	HRESULT CPlayer::GetTotalDurationTime(MFTIME* pDuration); // tiempo en centenares de nanosegundo que dura en total el video (total en segundos = retorno / 10000000). En caso de error devuelve -1
	HRESULT		  setAudioVolume(float audio_volume);

	// getter frame size. Si no se modifica específicamente se puede obtener el tamaño de frame nativo del video
	/// \param[out] height alto
	/// \param[out] width ancho
	void getFrameSize(int &height, int &width);

	// Frame control

	// Devuelve el número total de frames del video
	int obtenerNumeroDeFramesTotales(); // el rango "int" nos permite contemplar videos de hasta 9000 horas.

	// Obtiene número de frame según el tiempo de reproducción. El tiempo debe estar en segundos con decimales y estar en el rango válido del vidoe (mayor a 0 y menor a la duración)
	int obtenerNumeroDeFrameSegunTiempo(double tiempo);

	// Obtiene el tiempo de reproducción en el video del frame cuyo número se especifica en el parámetro, se puede usar para convertir frame a tiempo previo a hacer un salto de video
	double obtenerTiempoDeNumeroDeFrame(int numero_de_frame);

	// Devuelve el frame rate en frames cada 1000 segundos (29.97 FPS as an integer value of 29970)
	int getFrameRateInFramesPer1000Seconds();
	
	///////////////////////////////////////////////////////////////////////
	//  Name: SetPlaybackRate
	//  Description: 
	//      Gets the rate control service from Media Session.
	//      Sets the playback rate to the specified rate.
	//  Parameter:
	//      ** pMediaSession: [in] Media session object to query.** Media session has to be instantiated in the CPlayer class.
	//      rateRequested: [in] Playback rate to set.
	//      bThin: [in] Indicates whether to use thinning.
	///////////////////////////////////////////////////////////////////////
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms697048(v=vs.85).aspx
	HRESULT CPlayer::SetPlaybackRate(float rateRequested, BOOL bThin); // Set playback rate

	// Devuelve el playback rate. El normal es 1.0f.
	float getPlaybackRate();

	// Devuelve si en el playback rate se está aplicando thinning (submuestrando el video)
	bool getThinningPlaybackRate();

	// Indica si un playbackrate + thinning es soportado por el video (devuelve true) o no (devuelve false)
	bool isPlaybackRateSupported(float rateRequested, bool thinning, float* closest_supported_rate);

	// Devuelve la menor velocidad de reproducción soportada por el medio según sea con thinning o no en dirección hacia adelante o hacia atrás
	float getMinimumSupportedRate(MFRATE_DIRECTION direccion, BOOL thinning);

	// Apagar reproductor
    HRESULT       Shutdown();

	// Handler de evento de vodep
    HRESULT       HandleEvent(UINT_PTR pUnkPtr);
    
	// Query estado de reproducción
	PlayerState   GetState() const { return m_state; }

    // Video functionality

	// Repintar pantalla de video con frame actual
    HRESULT       Repaint();

	// Redimensionar pantalla de visualización
    HRESULT       ResizeVideo(WORD width, WORD height);
    
	// Query si tiene video
    BOOL          HasVideo() const { return (m_pVideoDisplay != NULL);  }

protected:
    
    // Constructor is private. Use static CreateInstance method to instantiate.
    CPlayer(HWND hVideo, HWND hEvent);

    // Destructor is private. Caller should call Release.
    virtual ~CPlayer(); 

    HRESULT Initialize();
    HRESULT CreateSession();
    HRESULT CloseSession();
    HRESULT StartPlayback();
	HRESULT SeekAndStartPlayback(int time); // tiempo en unidades de 100 nanosegundos desde el inicio del video

    // Media event handlers
    virtual HRESULT OnTopologyStatus(IMFMediaEvent *pEvent);
    virtual HRESULT OnPresentationEnded(IMFMediaEvent *pEvent);
    virtual HRESULT OnNewPresentation(IMFMediaEvent *pEvent);

    // Override to handle additional session events.
    virtual HRESULT OnSessionEvent(IMFMediaEvent*, MediaEventType) 
    { 
        return S_OK; 
    }

protected:
    long                    m_nRefCount;        // Reference count.

    IMFMediaSession         *m_pSession;
    IMFMediaSource          *m_pSource;
    IMFVideoDisplayControl  *m_pVideoDisplay;

    HWND                    m_hwndVideo;        // Video window.
    HWND                    m_hwndEvent;        // App window to receive events.
    PlayerState             m_state;            // Current state of the media session.
    HANDLE                  m_hCloseEvent;      // Event to wait on while closing.

private:

	// Add a source node to a topology.
	HRESULT AddSourceNode(	IMFTopology *pTopology,           // Topology.
							IMFMediaSource *pSource,          // Media source.
							IMFPresentationDescriptor *pPD,   // Presentation descriptor.
							IMFStreamDescriptor *pSD,         // Stream descriptor.
							IMFTopologyNode **ppNode);         // Receives the node pointer.

	//  Create an activation object for a renderer, based on the stream media type.
	HRESULT CreateMediaSinkActivate(	IMFStreamDescriptor *pSourceSD,     // Pointer to the stream descriptor.
										HWND hVideoWindow,                  // Handle to the video clipping window.
										IMFActivate **ppActivate);

	// Add an output node to a topology.
	HRESULT AddOutputNode(	IMFTopology *pTopology,     // Topology.
							IMFActivate *pActivate,     // Media sink activation object.
							DWORD dwId,                 // Identifier of the stream sink.
							IMFTopologyNode **ppNode);   // Receives the node pointer.

	//  Add a topology branch for one stream.
	//  For each stream, this function does the following:
	//    1. Creates a source node associated with the stream. 
	//    2. Creates an output node for the renderer. 
	//    3. Connects the two nodes.
	//  The media session will add any decoders that are needed.
	HRESULT AddBranchToPartialTopology(	IMFTopology *pTopology,         // Topology.
										IMFMediaSource *pSource,        // Media source.
										IMFPresentationDescriptor *pPD, // Presentation descriptor.
										DWORD iStream,                  // Stream index.
										HWND hVideoWnd);                 // Window for video playback.

	template <class Q> HRESULT GetEventObject(IMFMediaEvent *pEvent, Q **ppObject);

	//  Create a media source from a URL.
	HRESULT CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource);

	//  Create a playback topology from a media source.
	HRESULT CreatePlaybackTopology(IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, HWND hVideoWnd,IMFTopology **ppTopology);
};

#endif PLAYER_H
