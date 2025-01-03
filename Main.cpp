#include "Includes.h"

HINSTANCE Overlay::hInstance = nullptr;

INT APIENTRY WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PSTR lpCommandLine,
	_In_ INT nCmdShow
) {
	Overlay::hInstance = hInstance;

	const auto fnGetParentWindow = [ ] ( ) {
		return FindWindowA( "UnityWndClass", nullptr );
	};

	HWND hParentWindow = fnGetParentWindow( );
	Overlay overlay = Overlay( L"SAMPLE_CLASS_NAME", L"SAMPLE_WINDOW_NAME" );

	if ( !overlay.Init( ) ) {
		overlay.Destroy( );

		return 0;
	}

	overlay.Start( [ ] ( Overlay* pOverlay ) {
		static bool Open = true;
		static bool StreamProof = true;

		if ( GetAsyncKeyState( VK_INSERT ) & SHRT_MAX )
			Open = !Open;

		pOverlay->SetAbortInput( Open );
		pOverlay->SetStreamProof( StreamProof );

		if ( !Open )
			return;

		ImGui::Begin( "Sample" );
		{
			ImGui::Checkbox( "StreamProof", &StreamProof );
		}
		ImGui::End( );
	} );

	while ( true ) { Sleep( INT_MAX ); } // sleep so our app doesn't just exit....

	return 0;
};
