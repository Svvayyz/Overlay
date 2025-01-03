#include "Includes.h"

bool Overlay::Init( HWND& hParentWindow ) {
	if ( !_CreateWnd( hParentWindow ) ) return false;
	if ( !_ConfigDirectX( ) ) return false;
	if ( !_ConfigImGui( ) ) return false;

	return true;
}

void Overlay::Start( function<void( Overlay* )> fnRenderFunc ) {
	const auto thread = thread( [ this, fnRenderFunc ] ( ) {
		_Start( fnRenderFunc );
	} );

	thread.detach( );
}
void Overlay::_Start( function<void( Overlay* )> fnRenderFunc ) {
	while ( Running ) {
		_HandleMsg( ); // this function can change running, break if it did

		if ( !Running )
			break;

		Render( fnRenderFunc );
	}

	Destroy( );
}
void Overlay::Render( function<void( Overlay* )> fnRenderFunc ) {
	ImGui_ImplDX11_NewFrame( );
	ImGui_ImplWin32_NewFrame( );

	ImGui::NewFrame( );
	{
		fnRenderFunc( this );
	}
	ImGui::Render( );

	static float arrColor[ 4 ] = { 0.f, 0.f, 0.f, 0.f };

	m_pDeviceCtx->OMSetRenderTargets( 1U, &m_pTargetView, nullptr );
	m_pDeviceCtx->ClearRenderTargetView( m_pTargetView, arrColor );

	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

	m_pSwapChain->Present(
		0U,
		0U
	);
}
void Overlay::Destroy( ) {
	Running = false;

	if ( ImGui::GetCurrentContext( ) ) {
		ImGui_ImplDX11_Shutdown( );
		ImGui_ImplWin32_Shutdown( );

		ImGui::DestroyContext( );
	}

	if ( m_pDevice )
		m_pDevice->Release( );

	if ( m_pDeviceCtx )
		m_pDeviceCtx->Release( );

	if ( m_pSwapChain )
		m_pSwapChain->Release( );

	if ( m_pTargetView )
		m_pTargetView->Release( );

	if ( m_hWindow )
		DestroyWindow( m_hWindow );

	if ( m_szClassName && hInstance )
		UnregisterClassW( m_szClassName, hInstance );
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT CALLBACK Overlay::_WndProc( HWND hWindow, UINT uiMessage, WPARAM wParam, LPARAM lParam ) {
	if ( ImGui_ImplWin32_WndProcHandler( hWindow, uiMessage, wParam, lParam ) )
		return 0L;

	if ( uiMessage == WM_DESTROY ) {
		PostQuitMessage( 0 );

		return 0L;
	}

	return DefWindowProcW( hWindow, uiMessage, wParam, lParam );
}

void Overlay::_HandleMsg( ) {
	MSG msg;
	while ( PeekMessageW(
		&msg,
		nullptr,
		0U,
		0U,
		PM_REMOVE
	) )
	{
		TranslateMessage( &msg );
		DispatchMessageW( &msg );

		if ( msg.message == WM_QUIT )
		{
			Running = false;
		}
	}
}

bool Overlay::_CreateWnd( HWND& hParentWindow ) {
	WNDCLASSEXW Class {};
	Class.cbSize = sizeof( WNDCLASSEX );

	Class.style = CS_HREDRAW | CS_VREDRAW;
	Class.lpfnWndProc = _WndProc;
	Class.hInstance = hInstance;
	Class.lpszClassName = m_szClassName;

	RegisterClassExW( &Class );

	int iWidth = GetSystemMetrics( SM_CXSCREEN );
	int iHeight = GetSystemMetrics( SM_CYSCREEN );

	m_hWindow = CreateWindowExW(
		WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_COMPOSITED,
		m_szClassName,
		m_szWindowName,
		WS_POPUP,
		0,
		0,
		iWidth,
		iHeight,
		hParentWindow,
		nullptr,
		hInstance,
		nullptr
	);

	if ( !m_hWindow || m_hWindow == INVALID_HANDLE_VALUE )
		return false;

	// set the window to be transparent
	if ( !SetLayeredWindowAttributes( m_hWindow, RGB( 0, 0, 0 ), BYTE( 255 ), LWA_ALPHA ) )
		return false;

	// self explanatory 
	if ( !SetWindowDisplayAffinity( m_hWindow, WDA_EXCLUDEFROMCAPTURE ) )
		return false;

	RECT ClientRect {};
	if ( !GetClientRect( m_hWindow, &ClientRect ) )
		return false;

	RECT WindowRect {};
	if ( !GetWindowRect( m_hWindow, &WindowRect ) )
		return false;

	POINT Delta {};
	if ( !ClientToScreen( m_hWindow, &Delta ) )
		return false;

	MARGINS Margins = {
		WindowRect.left + ( Delta.x - WindowRect.left ),
		WindowRect.top + ( Delta.y - WindowRect.top ),
		ClientRect.right,
		ClientRect.bottom
	};

	// resize to fit the window
	if ( FAILED( DwmExtendFrameIntoClientArea( m_hWindow, &Margins ) ) )
		return false;

	return true;
}
bool Overlay::_ConfigDirectX( ) {
	DXGI_SWAP_CHAIN_DESC SwapChainDesc {};
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 1000U; //60U;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1U;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferCount = 2U;

	SwapChainDesc.OutputWindow = m_hWindow;
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	SwapChainDesc.SampleDesc.Count = 1U;

	D3D_FEATURE_LEVEL Levels[ 2 ] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL Level;

	if ( FAILED( D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		Levels,
		sizeof( Levels ) / sizeof( D3D_FEATURE_LEVEL ),
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&m_pSwapChain,
		&m_pDevice,
		&Level,
		&m_pDeviceCtx
	) ) )
		return false;

	ID3D11Texture2D* pChainBuffer = nullptr;
	if ( FAILED( m_pSwapChain->GetBuffer( 0U, IID_PPV_ARGS( &pChainBuffer ) ) ) )
		return false;

	if ( !pChainBuffer )
		return false;

	if ( FAILED( m_pDevice->CreateRenderTargetView(
		pChainBuffer,
		nullptr,
		&m_pTargetView
	) ) )
		return false;

	pChainBuffer->Release( );

	ShowWindow( m_hWindow, SW_SHOW );
	UpdateWindow( m_hWindow );

	return true;
}
bool Overlay::_ConfigImGui( ) {
	ImGui::CreateContext( );
	ImGui::StyleColorsDark( );

	ImGuiIO& io = ImGui::GetIO( );
	io.IniFilename = NULL;
	io.LogFilename = NULL;

  // set up imgui here...

	if ( !ImGui_ImplWin32_Init( m_hWindow ) )
		return false;

	if ( !ImGui_ImplDX11_Init( m_pDevice, m_pDeviceCtx ) )
		return false;

	return true;
}
