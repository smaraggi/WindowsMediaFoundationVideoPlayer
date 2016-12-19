// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// How to play movies with Media Files Foundation: 
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms703190(v=vs.85).aspx

// Steps to use CPlayer
// 1 https://msdn.microsoft.com/en-us/library/windows/desktop/dd979592(v=vs.85).aspx
// 2 https://msdn.microsoft.com/en-us/library/windows/desktop/dd979593(v=vs.85).aspx
// 3 https://msdn.microsoft.com/en-us/library/windows/desktop/dd979594(v=vs.85).aspx
// 4 https://msdn.microsoft.com/en-us/library/windows/desktop/dd979595(v=vs.85).aspx
// 5 https://msdn.microsoft.com/en-us/library/windows/desktop/dd979596(v=vs.85).aspx
// 6 https://msdn.microsoft.com/en-us/library/windows/desktop/dd979597(v=vs.85).aspx
// 7 https://msdn.microsoft.com/en-us/library/windows/desktop/dd979598(v=vs.85).aspx
// Example https://msdn.microsoft.com/en-us/library/windows/desktop/ff728866(v=vs.85).aspx

// Seeking Player / Utilización de IMFPresentationClock
// https://msdn.microsoft.com/en-us/library/windows/desktop/ee892373(v=vs.85).aspx
// File duration / seeking:
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd389281(v=vs.85).aspx#getting_file_duration
// Presentation Sample
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms703153(v=vs.85).aspx

// Get Frame Rate
// http://stackoverflow.com/questions/19211610/how-can-i-read-the-info-of-a-media-file-using-visual-c
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb762197(v=vs.85).aspx

// Frame Number in video files
// https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/41b62ea7-dd6c-4963-b251-280d2f075c71/is-it-possible-to-get-the-number-of-video-frames-for-a-file-using-the-source-reader?forum=mediafoundationdevelopment
// Entonces...
// ...obtener frame rate con API Media Foundation
// ... relacionar tiempo con número de frame y rezar que de lo mismo que SDK Windows Media Player en "frame mode".
// Ver propiedad PKEY_Video_FrameRate de API de Windows

#include "stdafx.h"

#include "CPlayer.h"
#include <assert.h>

#include <Mfobjects.h>
#include <Mfidl.h>

#include <propkey.h>

#pragma comment(lib, "shlwapi")

const MFTIME ONE_SECOND = 10000000; // One second.
const LONG   ONE_MSEC = 1000;       // One millisecond

// Add a source node to a topology.
HRESULT CPlayer::AddSourceNode(
    IMFTopology *pTopology,           // Topology.
    IMFMediaSource *pSource,          // Media source.
    IMFPresentationDescriptor *pPD,   // Presentation descriptor.
    IMFStreamDescriptor *pSD,         // Stream descriptor.
    IMFTopologyNode **ppNode)         // Receives the node pointer.
{
    IMFTopologyNode *pNode = NULL;

    // Create the node.
    HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);
    if (FAILED(hr))
    {
        goto done;
    }

    // Set the attributes.
    hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
    if (FAILED(hr))
    {
        goto done;
    }
    
    // Add the node to the topology.
    hr = pTopology->AddNode(pNode);
    if (FAILED(hr))
    {
        goto done;
    }

    // Return the pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

done:
    SafeRelease(&pNode);
    return hr;
}

//  Create an activation object for a renderer, based on the stream media type.
HRESULT CPlayer::CreateMediaSinkActivate(
    IMFStreamDescriptor *pSourceSD,     // Pointer to the stream descriptor.
    HWND hVideoWindow,                  // Handle to the video clipping window.
    IMFActivate **ppActivate
)
{
    IMFMediaTypeHandler *pHandler = NULL;
    IMFActivate *pActivate = NULL;

    // Get the media type handler for the stream.
    HRESULT hr = pSourceSD->GetMediaTypeHandler(&pHandler);
    if (FAILED(hr))
    {
        goto done;
    }

    // Get the major media type.
    GUID guidMajorType;
    hr = pHandler->GetMajorType(&guidMajorType);
    if (FAILED(hr))
    {
        goto done;
    }
 
    // Create an IMFActivate object for the renderer, based on the media type.
    if (MFMediaType_Audio == guidMajorType)
    {
        // Create the audio renderer.
        hr = MFCreateAudioRendererActivate(&pActivate);
    }
    else if (MFMediaType_Video == guidMajorType)
    {
        // Create the video renderer.
        hr = MFCreateVideoRendererActivate(hVideoWindow, &pActivate);
    }
    else
    {
        // Unknown stream type. 
        hr = E_FAIL;
        // Optionally, you could deselect this stream instead of failing.
    }
    if (FAILED(hr))
    {
        goto done;
    }
 
    // Return IMFActivate pointer to caller.
    *ppActivate = pActivate;
    (*ppActivate)->AddRef();

done:
    SafeRelease(&pHandler);
    SafeRelease(&pActivate);
    return hr;
}

// Add an output node to a topology.
HRESULT CPlayer::AddOutputNode(
    IMFTopology *pTopology,     // Topology.
    IMFActivate *pActivate,     // Media sink activation object.
    DWORD dwId,                 // Identifier of the stream sink.
    IMFTopologyNode **ppNode)   // Receives the node pointer.
{
    IMFTopologyNode *pNode = NULL;

    // Create the node.
    HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);
    if (FAILED(hr))
    {
        goto done;
    }

    // Set the object pointer.
    hr = pNode->SetObject(pActivate);
    if (FAILED(hr))
    {
        goto done;
    }

    // Set the stream sink ID attribute.
    hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
    if (FAILED(hr))
    {
        goto done;
    }

    // Add the node to the topology.
    hr = pTopology->AddNode(pNode);
    if (FAILED(hr))
    {
        goto done;
    }

    // Return the pointer to the caller.
    *ppNode = pNode;
    (*ppNode)->AddRef();

done:
    SafeRelease(&pNode);
    return hr;
}

//  Add a topology branch for one stream.
//
//  For each stream, this function does the following:
//
//    1. Creates a source node associated with the stream. 
//    2. Creates an output node for the renderer. 
//    3. Connects the two nodes.
//
//  The media session will add any decoders that are needed.

HRESULT CPlayer::AddBranchToPartialTopology(
    IMFTopology *pTopology,         // Topology.
    IMFMediaSource *pSource,        // Media source.
    IMFPresentationDescriptor *pPD, // Presentation descriptor.
    DWORD iStream,                  // Stream index.
    HWND hVideoWnd)                 // Window for video playback.
{
    IMFStreamDescriptor *pSD = NULL;
    IMFActivate         *pSinkActivate = NULL;
    IMFTopologyNode     *pSourceNode = NULL;
    IMFTopologyNode     *pOutputNode = NULL;

    BOOL fSelected = FALSE;

    HRESULT hr = pPD->GetStreamDescriptorByIndex(iStream, &fSelected, &pSD);
    if (FAILED(hr))
    {
        goto done;
    }

    if (fSelected)
    {
        // Create the media sink activation object.
        hr = CreateMediaSinkActivate(pSD, hVideoWnd, &pSinkActivate);
        if (FAILED(hr))
        {
            goto done;
        }

        // Add a source node for this stream.
        hr = AddSourceNode(pTopology, pSource, pPD, pSD, &pSourceNode);
        if (FAILED(hr))
        {
            goto done;
        }

        // Create the output node for the renderer.
        hr = AddOutputNode(pTopology, pSinkActivate, 0, &pOutputNode);
        if (FAILED(hr))
        {
            goto done;
        }

        // Connect the source node to the output node.
        hr = pSourceNode->ConnectOutput(0, pOutputNode, 0);
    }
    // else: If not selected, don't add the branch. 

done:
    SafeRelease(&pSD);
    SafeRelease(&pSinkActivate);
    SafeRelease(&pSourceNode);
    SafeRelease(&pOutputNode);
    return hr;
}

template <class Q> HRESULT CPlayer::GetEventObject(IMFMediaEvent *pEvent, Q **ppObject)
{
    *ppObject = NULL;   // zero output

    PROPVARIANT var;
    HRESULT hr = pEvent->GetValue(&var);
    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_UNKNOWN)
        {
            hr = var.punkVal->QueryInterface(ppObject);
        }
        else
        {
            hr = MF_E_INVALIDTYPE;
        }
        PropVariantClear(&var);
    }
    return hr;
}

//  Create a media source from a URL.
HRESULT CPlayer::CreateMediaSource(PCWSTR sURL, IMFMediaSource **ppSource)
{
    MF_OBJECT_TYPE ObjectType = MF_OBJECT_INVALID;

    IMFSourceResolver* pSourceResolver = NULL;
    IUnknown* pSource = NULL;

    // Create the source resolver.
    HRESULT hr = MFCreateSourceResolver(&pSourceResolver);
    if (FAILED(hr))
    {
        goto done;
    }

    // Use the source resolver to create the media source.

    // Note: For simplicity this sample uses the synchronous method to create 
    // the media source. However, creating a media source can take a noticeable
    // amount of time, especially for a network source. For a more responsive 
    // UI, use the asynchronous BeginCreateObjectFromURL method.

    hr = pSourceResolver->CreateObjectFromURL(
        sURL,                       // URL of the source.
        MF_RESOLUTION_MEDIASOURCE,  // Create a source object.
        NULL,                       // Optional property store.
        &ObjectType,        // Receives the created object type. 
        &pSource            // Receives a pointer to the media source.
        );
    if (FAILED(hr))
    {
        goto done;
    }

    // Get the IMFMediaSource interface from the media source.
    hr = pSource->QueryInterface(IID_PPV_ARGS(ppSource));

done:
    SafeRelease(&pSourceResolver);
    SafeRelease(&pSource);
    return hr;
}

//  Create a playback topology from a media source.
HRESULT CPlayer::CreatePlaybackTopology(IMFMediaSource *pSource, IMFPresentationDescriptor *pPD, HWND hVideoWnd,IMFTopology **ppTopology)
{
    IMFTopology *pTopology = NULL;
    DWORD cSourceStreams = 0;

    // Create a new topology.
    HRESULT hr = MFCreateTopology(&pTopology);
    if (FAILED(hr))
    {
        goto done;
    }

    // Get the number of streams in the media source.
    hr = pPD->GetStreamDescriptorCount(&cSourceStreams);
    if (FAILED(hr))
    {
        goto done;
    }

    // For each stream, create the topology nodes and add them to the topology.
    for (DWORD i = 0; i < cSourceStreams; i++)
    {
        hr = AddBranchToPartialTopology(pTopology, pSource, pPD, i, hVideoWnd);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    // Return the IMFTopology pointer to the caller.
    *ppTopology = pTopology;
    (*ppTopology)->AddRef();

done:
    SafeRelease(&pTopology);
    return hr;
}
//  Static class method to create the CPlayer object.

HRESULT CPlayer::CreateInstance(
    HWND hVideo,                  // Video window.
    HWND hEvent,                  // Window to receive notifications.
    CPlayer **ppPlayer)           // Receives a pointer to the CPlayer object.
{
    if (ppPlayer == NULL)
    {
        return E_POINTER;
    }

    CPlayer *pPlayer = new (std::nothrow) CPlayer(hVideo, hEvent);
    if (pPlayer == NULL)
    {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pPlayer->Initialize();
    if (SUCCEEDED(hr))
    {
        *ppPlayer = pPlayer;
    }
    else
    {
        pPlayer->Release();
    }
    return hr;
}

HRESULT CPlayer::Initialize()
{
    // Start up Media Foundation platform.
    HRESULT hr = MFStartup(MF_VERSION);
    if (SUCCEEDED(hr))
    {
        m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hCloseEvent == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }
    return hr;
}

CPlayer::CPlayer(HWND hVideo, HWND hEvent) : 
    m_pSession(NULL),
    m_pSource(NULL),
    m_pVideoDisplay(NULL),
    m_hwndVideo(hVideo),
    m_hwndEvent(hEvent),
    m_state(Closed),
    m_hCloseEvent(NULL),
    m_nRefCount(1)
{
}

CPlayer::~CPlayer()
{
    assert(m_pSession == NULL);  
    // If FALSE, the app did not call Shutdown().

    // When CPlayer calls IMediaEventGenerator::BeginGetEvent on the
    // media session, it causes the media session to hold a reference 
    // count on the CPlayer. 
    
    // This creates a circular reference count between CPlayer and the 
    // media session. Calling Shutdown breaks the circular reference 
    // count.

    // If CreateInstance fails, the application will not call 
    // Shutdown. To handle that case, call Shutdown in the destructor. 

    Shutdown();
}

// IUnknown methods

HRESULT CPlayer::QueryInterface(REFIID riid, void** ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CPlayer, IMFAsyncCallback),
        { 0 }
    };
    return QISearch(this, qit, riid, ppv);
}

ULONG CPlayer::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

ULONG CPlayer::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0)
    {
        delete this;
    }
    return uCount;
}

//  Open a URL for playback.
HRESULT CPlayer::OpenURL(const WCHAR *sURL)
{
    // 1. Create a new media session.
    // 2. Create the media source.
    // 3. Create the topology.
    // 4. Queue the topology [asynchronous]
    // 5. Start playback [asynchronous - does not happen in this method.]

    IMFTopology *pTopology = NULL;
    IMFPresentationDescriptor* pSourcePD = NULL;

    // 1. Create the media session.
    HRESULT hr = CreateSession();
    if (FAILED(hr))
    {
        goto done;
    }

    // 2. Create the media source.
    hr = CreateMediaSource(sURL, &m_pSource);
    if (FAILED(hr))
    {
        goto done;
    }

    // Create the presentation descriptor for the media source.
    hr = m_pSource->CreatePresentationDescriptor(&pSourcePD);
    if (FAILED(hr))
    {
        goto done;
    }

    // 3. Create a partial topology.
    hr = CreatePlaybackTopology(m_pSource, pSourcePD, m_hwndVideo, &pTopology);
    if (FAILED(hr))
    {
        goto done;
    }

    // 4. Set the topology on the media session.
    hr = m_pSession->SetTopology(0, pTopology);
    if (FAILED(hr))
    {
        goto done;
    }

    m_state = OpenPending;

    // If SetTopology succeeds, the media session will queue an 
    // MESessionTopologySet event.

done:
    if (FAILED(hr))
    {
        m_state = Closed;
    }

    SafeRelease(&pSourcePD);
    SafeRelease(&pTopology);
    return hr;
}

//  Pause playback.
HRESULT CPlayer::Pause()    
{
    if (m_state != Started)
    {
        return MF_E_INVALIDREQUEST;
    }
    if (m_pSession == NULL || m_pSource == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pSession->Pause();
    if (SUCCEEDED(hr))
    {
        m_state = Paused;
    }

    return hr;
}

// Stop playback.
HRESULT CPlayer::Stop()
{
    if (m_state != Started && m_state != Paused)
    {
        return MF_E_INVALIDREQUEST;
    }
    if (m_pSession == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT hr = m_pSession->Stop();
    if (SUCCEEDED(hr))
    {
        m_state = Stopped;
    }
    return hr;
}

HRESULT CPlayer::SeekAndPlay(int time)
{
	return this->SeekAndStartPlayback(time);
}

//  Repaint the video window. Call this method on WM_PAINT.

HRESULT CPlayer::Repaint()
{
    if (m_pVideoDisplay)
    {
        return m_pVideoDisplay->RepaintVideo();
    }
    else
    {
        return S_OK;
    }
}

//  Resize the video rectangle.
//
//  Call this method if the size of the video window changes.

HRESULT CPlayer::ResizeVideo(WORD width, WORD height)
{
    if (m_pVideoDisplay)
    {
        // Set the destination rectangle.
        // Leave the default source rectangle (0,0,1,1).

        RECT rcDest = { 0, 0, width, height };

        return m_pVideoDisplay->SetVideoPosition(NULL, &rcDest);
    }
    else
    {
        return S_OK;
    }
}

//  Callback for the asynchronous BeginGetEvent method.

HRESULT CPlayer::Invoke(IMFAsyncResult *pResult)
{
    MediaEventType meType = MEUnknown;  // Event type

    IMFMediaEvent *pEvent = NULL;

    // Get the event from the event queue.
    HRESULT hr = m_pSession->EndGetEvent(pResult, &pEvent);
    if (FAILED(hr))
    {
        goto done;
    }

    // Get the event type. 
    hr = pEvent->GetType(&meType);
    if (FAILED(hr))
    {
        goto done;
    }

    if (meType == MESessionClosed)
    {
        // The session was closed. 
        // The application is waiting on the m_hCloseEvent event handle. 
        SetEvent(m_hCloseEvent);
    }
    else
    {
        // For all other events, get the next event in the queue.
        hr = m_pSession->BeginGetEvent(this, NULL);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    // Check the application state. 
        
    // If a call to IMFMediaSession::Close is pending, it means the 
    // application is waiting on the m_hCloseEvent event and
    // the application's message loop is blocked. 

    // Otherwise, post a private window message to the application. 

    if (m_state != Closing)
    {
        // Leave a reference count on the event.
        pEvent->AddRef();

		// Post the event with a message to the video window, si it can be handled in HandleEvent method afterwards
        PostMessage(m_hwndEvent, WM_APP_PLAYER_EVENT, (WPARAM)pEvent, (LPARAM)meType);
    }

done:
    SafeRelease(&pEvent);
    return S_OK;
}

HRESULT CPlayer::HandleEvent(UINT_PTR pEventPtr)
{
    HRESULT hrStatus = S_OK;            
    MediaEventType meType = MEUnknown;  

    IMFMediaEvent *pEvent = (IMFMediaEvent*)pEventPtr;

    if (pEvent == NULL)
    {
        return E_POINTER;
    }

    // Get the event type.
    HRESULT hr = pEvent->GetType(&meType);
    if (FAILED(hr))
    {
        goto done;
    }

    // Get the event status. If the operation that triggered the event 
    // did not succeed, the status is a failure code.
    hr = pEvent->GetStatus(&hrStatus);

    // Check if the async operation succeeded.
    if (SUCCEEDED(hr) && FAILED(hrStatus)) 
    {
        hr = hrStatus;
    }
    if (FAILED(hr))
    {
        goto done;
    }

    switch(meType)
    {
    case MESessionTopologyStatus:
        hr = OnTopologyStatus(pEvent);
        break;

    case MEEndOfPresentation:
        hr = OnPresentationEnded(pEvent);
        break;

    case MENewPresentation:
        hr = OnNewPresentation(pEvent);
        break;

	case MENewStream:
		hr = 1000;
		break;

	// El evento MEConnectEnd se recibe en un SourceOpenMonitor, no en esta clase
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms698971(v=vs.85).aspx

    default:
        hr = OnSessionEvent(pEvent, meType);
        break;
    }

done:
    SafeRelease(&pEvent);
    return hr;
}

//  Release all resources held by this object.
HRESULT CPlayer::Shutdown()
{
    // Close the session
    HRESULT hr = CloseSession();

    // Shutdown the Media Foundation platform
    MFShutdown();

    if (m_hCloseEvent)
    {
        CloseHandle(m_hCloseEvent);
        m_hCloseEvent = NULL;
    }

    return hr;
}

/// Protected methods

HRESULT CPlayer::OnTopologyStatus(IMFMediaEvent *pEvent)
{
    UINT32 status; 

    HRESULT hr = pEvent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status);
    if (SUCCEEDED(hr) && (status == MF_TOPOSTATUS_READY))
    {
        SafeRelease(&m_pVideoDisplay);

        // Get the IMFVideoDisplayControl interface from EVR. This call is
        // expected to fail if the media file does not have a video stream.

        (void)MFGetService(m_pSession, MR_VIDEO_RENDER_SERVICE, IID_PPV_ARGS(&m_pVideoDisplay));

        // hr = StartPlayback(); // No queremos que inicie la reproducción del video al terminar la carga del mismo
		m_state = Stopped;
    }
    return hr;
}


//  Handler for MEEndOfPresentation event.
HRESULT CPlayer::OnPresentationEnded(IMFMediaEvent *pEvent)
{
    // The session puts itself into the stopped state automatically.
    m_state = Stopped;
    return S_OK;
}

//  Handler for MENewPresentation event.
//
//  This event is sent if the media source has a new presentation, which 
//  requires a new topology. 

HRESULT CPlayer::OnNewPresentation(IMFMediaEvent *pEvent)
{
    IMFPresentationDescriptor *pPD = NULL;
    IMFTopology *pTopology = NULL;

    // Get the presentation descriptor from the event.
    HRESULT hr = GetEventObject(pEvent, &pPD);
    if (FAILED(hr))
    {
        goto done;
    }

    // Create a partial topology.
    hr = CreatePlaybackTopology(m_pSource, pPD,  m_hwndVideo,&pTopology);
    if (FAILED(hr))
    {
        goto done;
    }

    // Set the topology on the media session.
    hr = m_pSession->SetTopology(0, pTopology);
    if (FAILED(hr))
    {
        goto done;
    }

    m_state = OpenPending;

done:
    SafeRelease(&pTopology);
    SafeRelease(&pPD);
    return S_OK;
}

//  Create a new instance of the media session.
HRESULT CPlayer::CreateSession()
{
    // Close the old session, if any.
    HRESULT hr = CloseSession();
    if (FAILED(hr))
    {
        goto done;
    }

    assert(m_state == Closed);

    // Create the media session.
    hr = MFCreateMediaSession(NULL, &m_pSession);
    if (FAILED(hr))
    {
        goto done;
    }

    // Start pulling events from the media session
    hr = m_pSession->BeginGetEvent((IMFAsyncCallback*)this, NULL);
    if (FAILED(hr))
    {
        goto done;
    }

    m_state = Ready;

done:
    return hr;
}

//  Close the media session. 
HRESULT CPlayer::CloseSession()
{
    //  The IMFMediaSession::Close method is asynchronous, but the 
    //  CPlayer::CloseSession method waits on the MESessionClosed event.
    //  
    //  MESessionClosed is guaranteed to be the last event that the 
    //  media session fires.

    HRESULT hr = S_OK;

    SafeRelease(&m_pVideoDisplay);

    // First close the media session.
    if (m_pSession)
    {
        DWORD dwWaitResult = 0;

        m_state = Closing;
           
        hr = m_pSession->Close();
        // Wait for the close operation to complete
        if (SUCCEEDED(hr))
        {
            dwWaitResult = WaitForSingleObject(m_hCloseEvent, 5000);
            if (dwWaitResult == WAIT_TIMEOUT)
            {
                assert(FALSE);
            }
            // Now there will be no more events from this session.
        }
    }

    // Complete shutdown operations.
    if (SUCCEEDED(hr))
    {
        // Shut down the media source. (Synchronous operation, no events.)
        if (m_pSource)
        {
            (void)m_pSource->Shutdown();
        }
        // Shut down the media session. (Synchronous operation, no events.)
        if (m_pSession)
        {
            (void)m_pSession->Shutdown();
        }
    }

    SafeRelease(&m_pSource);
    SafeRelease(&m_pSession);
    m_state = Closed;
    return hr;
}

//  Start playback from the current position. 
HRESULT CPlayer::StartPlayback()
{
    assert(m_pSession != NULL);

    PROPVARIANT varStart;
    PropVariantInit(&varStart);

    HRESULT hr = m_pSession->Start(&GUID_NULL, &varStart);
    if (SUCCEEDED(hr))
    {
        // Note: Start is an asynchronous operation. However, we
        // can treat our state as being already started. If Start
        // fails later, we'll get an MESessionStarted event with
        // an error code, and we will update our state then.
        m_state = Started;
    }
    PropVariantClear(&varStart);
    return hr;
}

HRESULT CPlayer::SeekAndStartPlayback(int time)
{
	// time viene en unidades múltiplo de 100 nanosegundos

	assert(m_pSession != NULL);

    PROPVARIANT varStart;
    PropVariantInit(&varStart);

	// TODO setear parámetros de inicio...
	varStart.vt = VT_I8;
	varStart.hVal.QuadPart = time;

    HRESULT hr = m_pSession->Start(&GUID_NULL, &varStart);
    if (SUCCEEDED(hr))
    {
        // Note: Start is an asynchronous operation. However, we
        // can treat our state as being already started. If Start
        // fails later, we'll get an MESessionStarted event with
        // an error code, and we will update our state then.
        m_state = Started;
    }
    PropVariantClear(&varStart);
    return hr;
}

HRESULT CPlayer::GetTotalDurationTime(MFTIME* pDuration)
{
	*pDuration = 0;

    IMFPresentationDescriptor *pPD = NULL;

    HRESULT hr = this->m_pSource->CreatePresentationDescriptor(&pPD);
    if (SUCCEEDED(hr))
    {
        hr = pPD->GetUINT64(MF_PD_DURATION, (UINT64*)pDuration);
        pPD->Release();
    }

	return hr;
}

double CPlayer::getActualPlayingTime()
{
	// Devuelve el playing time en segundos con número decimal... (5,4 segundos por ejemplo)
	double retorno = -1;

	IMFClock* pClock;
	HRESULT hr = this->m_pSession->GetClock(&pClock); // Presentation Clock

	if (SUCCEEDED(hr))
    {
		IMFPresentationClock* presentation_clock = static_cast<IMFPresentationClock*>(pClock);

		MFCLOCK_STATE pcClockState;
		hr = presentation_clock->GetState(0, &pcClockState);
	
		if (SUCCEEDED(hr) && MFCLOCK_STATE_INVALID != pcClockState)
		{	
			MFTIME time;
			presentation_clock->GetTime(&time);

			retorno = (double)(time) / 10000000; // 
		}
	}
	
	return retorno;
}

//  Start playback from paused or stopped.
HRESULT CPlayer::Play()
{
    if (m_state != Paused && m_state != Stopped)
    {
        return MF_E_INVALIDREQUEST;
    }
    if (m_pSession == NULL || m_pSource == NULL)
    {
        return E_UNEXPECTED;
    }
    return StartPlayback();
}

//</SnippetPlayer.cpp>

HRESULT CPlayer::SetPlaybackRate(float rateRequested, BOOL bThin)
{
	IMFMediaSession *pMediaSession = this->m_pSession;
	HRESULT hr = E_FAIL;

	if (pMediaSession)
	{
		hr = S_OK;
		IMFRateControl *pRateControl = NULL;

		// Get the rate control object from the Media Session.
		hr = MFGetService(pMediaSession, MF_RATE_CONTROL_SERVICE, IID_IMFRateControl, (void**) &pRateControl ); 

		// Set the playback rate.
		if(SUCCEEDED(hr))
		{
			hr = pRateControl->SetRate( bThin, rateRequested); 
		}

		// Clean up.
		// SAFE_RELEASE(pRateControl );
		if (pRateControl)
		{
			pRateControl->Release();
			pRateControl = NULL;
		}
	}

    return hr;
}


float CPlayer::getPlaybackRate()
{
	IMFRateControl *pRateControl = NULL;

	float playback_rate = 0.0;
	BOOL playback_thinning;

	if (this->m_pSession)
	{
		HRESULT hr = MFGetService(this->m_pSession, MF_RATE_CONTROL_SERVICE, IID_IMFRateControl, (void**) &pRateControl );

		hr = pRateControl->GetRate(&playback_thinning, &playback_rate);
	}

	return playback_rate;
}

bool CPlayer::getThinningPlaybackRate()
{	
	IMFRateControl *pRateControl = NULL;

	float playback_rate = 0.0;
	BOOL playback_thinning;

	if (this->m_pSession)
	{
		HRESULT hr = MFGetService(this->m_pSession, MF_RATE_CONTROL_SERVICE, IID_IMFRateControl, (void**) &pRateControl );

		hr = pRateControl->GetRate(&playback_thinning, &playback_rate);
	}

	return playback_thinning;
}

bool CPlayer::isPlaybackRateSupported(float rateRequested, bool thinning, float* closest_supported_rate)
{
	IMFRateSupport *pRateSupport = NULL;
	bool rate_supported = false;

	if (this->m_pSession)
	{
		HRESULT hr = MFGetService(this->m_pSession, MF_RATE_CONTROL_SERVICE, IID_IMFRateSupport, (void**) &pRateSupport );

		hr = pRateSupport->IsRateSupported(thinning, rateRequested, closest_supported_rate);

		if (hr == S_OK)
		{
			rate_supported = true;
		}
		if (hr == MF_E_THINNING_UNSUPPORTED)
		{
			// ya estaba en false, se puede diferenciar alguno de estos 3 casos...
			int a = 0; // add breakpoint for debug purposes
		}
		else if (hr == MF_E_UNSUPPORTED_RATE)
		{
			// false...
			int a = 0; // add breakpoint for debug purposes
		}
		else if (hr == MF_E_REVERSE_UNSUPPORTED)
		{
			// false...
			int a = 0; // add breakpoint for debug purposes
		}
		else
		{
			// false...
			int a = 0; // add breakpoint for debug purposes
		}
	}

	return rate_supported;
}

float CPlayer::getMinimumSupportedRate(MFRATE_DIRECTION direccion, BOOL thinning)
{
	IMFRateSupport *pRateSupport = NULL;

	float minimum_rate = 0.0;

	if (this->m_pSession)
	{
		HRESULT hr = MFGetService(this->m_pSession, MF_RATE_CONTROL_SERVICE, IID_IMFRateSupport, (void**) &pRateSupport );

		hr = pRateSupport->GetSlowestRate(direccion, thinning, &minimum_rate);
	}

	return minimum_rate;
}

HRESULT CPlayer::setAudioVolume(float audio_volume)
{
	HRESULT hr = E_FAIL;

	if (this->m_pSession)
	{
		IMFAudioStreamVolume* audio_volume_service = NULL;

		HRESULT hr = MFGetService(this->m_pSession, MR_STREAM_VOLUME_SERVICE, IID_IMFAudioStreamVolume, (void**) &audio_volume_service);

		if (audio_volume_service && S_OK == hr)
		{
			UINT32 cantidad_canales = 0;
			
			hr = audio_volume_service->GetChannelCount(&cantidad_canales);

			if (S_OK == hr)
			{				
				for (int x = 0; x < cantidad_canales && S_OK == hr; x++)
				{
					hr = audio_volume_service->SetChannelVolume(x, audio_volume);
				}
			}
		}
	}

	return hr;
}

int CPlayer::obtenerNumeroDeFramesTotales()
{
	MFTIME duration;
	this->GetTotalDurationTime(&duration);

	double tiempo_total_en_segundos = duration / 10000000.0;

	int numero_frame = this->obtenerNumeroDeFrameSegunTiempo(tiempo_total_en_segundos);

	return numero_frame;
}

int CPlayer::obtenerNumeroDeFrameSegunTiempo(double tiempo)
{	
	int fp1000s = this->getFrameRateInFramesPer1000Seconds();

	double tiempo_un_cuadro = 1000.0 / fp1000s; // espacio de tiempo que corresponde a un cuadro en segundos

	double cantidad_cuadros_avanzados = tiempo / tiempo_un_cuadro; // cantidad de cuadros avanzados

	int numero_cuadro = cantidad_cuadros_avanzados; // el número de cuadro trunca el valor anterior, puesto que los saltos de cuadro son discretos

	return numero_cuadro;
}

double CPlayer::obtenerTiempoDeNumeroDeFrame(int numero_de_frame)
{
	int fp1000s = this->getFrameRateInFramesPer1000Seconds();

	double tiempo_frame = numero_de_frame * 1000.0 / fp1000s;

	return tiempo_frame;
}

int CPlayer::getFrameRateInFramesPer1000Seconds()
{
	int frames_per_1000_seconds = 0;

	IMFMediaSource* pSource = this->m_pSource;

	if (pSource)
	{
		IPropertyStore *pProps = NULL;

		HRESULT hr = MFGetService(pSource, MF_PROPERTY_HANDLER_SERVICE, IID_PPV_ARGS(&pProps));

		if (SUCCEEDED(hr))
		{
			PROPVARIANT pv;

			hr = pProps->GetValue(PKEY_Video_FrameRate, &pv);

			frames_per_1000_seconds = pv.intVal;

			pProps->Release();
		}
	}

	// Devuelve cantidad de frames cada 1000 segundos. 30FPS retorna 30000 frames cada 1000 segundos...
	return frames_per_1000_seconds;
}

void CPlayer::getFrameSize(int &height, int &width)
{
	IMFMediaSource* pSource = this->m_pSource;

	if (pSource)
	{
		IPropertyStore *pProps = NULL;

		HRESULT hr = MFGetService(pSource, MF_PROPERTY_HANDLER_SERVICE, IID_PPV_ARGS(&pProps));

		if (SUCCEEDED(hr))
		{
			// Altura
			PROPVARIANT pvHeight;
			hr = pProps->GetValue(PKEY_Video_FrameHeight, &pvHeight);
			height = pvHeight.intVal;

			// Ancho
			PROPVARIANT pvWidth;
			hr = pProps->GetValue(PKEY_Video_FrameWidth, &pvWidth);
			width = pvWidth.intVal;

			pProps->Release();

		}
	}
}