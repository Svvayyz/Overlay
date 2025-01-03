#pragma once

class Overlay {
public:
	static HINSTANCE hInstance;

public:
	Overlay( ) : m_szClassName( L"" ), m_szWindowName( L"" ) { }
	Overlay( LPCWSTR szClassName, LPCWSTR szWindowName )
		: m_szClassName( szClassName ), m_szWindowName( szWindowName ) {}

public:
	bool Init( HWND& hParentWindow );

	void Start( function<void( Overlay* )> fnRenderFunc );
	void Render( function<void( Overlay* )>& fnRenderFunc );
	void Destroy( );

private:
       void _Start( function<void( Overlay* )>& fnRenderFunc );

public:
	bool Running = true;
	bool AbortInput = true;
	bool StreamProof = true;

	void SetAbortInput( bool bAbortInput ) {
		SetWindowLongW(
			m_hWindow,
			GWL_EXSTYLE,
			bAbortInput ? WS_EX_TOOLWINDOW | WS_EX_LAYERED : WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_COMPOSITED
		);

		AbortInput = bAbortInput;
	}
	void SetStreamProof( bool bStreamProof ) {
		SetWindowDisplayAffinity( m_hWindow, bStreamProof ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE );

		StreamProof = bStreamProof;
	}

private:
	static LRESULT CALLBACK _WndProc( HWND hWindow, UINT uiMessage, WPARAM wParam, LPARAM lParam );
	void _HandleMsg( );

	bool _CreateWnd( HWND& hParentWindow );
	bool _ConfigDirectX( );
	bool _ConfigImGui( );

private:
	HWND m_hWindow;

	LPCWSTR m_szWindowName;
	LPCWSTR m_szClassName;

	ID3D11Device* m_pDevice;
	ID3D11DeviceContext* m_pDeviceCtx;
	IDXGISwapChain* m_pSwapChain;
	ID3D11RenderTargetView* m_pTargetView;
};

