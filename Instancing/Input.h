#pragma once
#define DIRECTINPUT_VERSION     0x0800
#define INITGUID
#include <dinput.h>
#include <Xinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

#define PJX1_LXPOS				0x0200
#define PJX1_LYPOS				0x0201
#define PJX1_LTRG				0x0202
#define PJX1_RXPOS				0x0203
#define PJX1_RYPOS				0x0204
#define PJX1_RTRG				0x0205
#define PJX1_BTNS				0x0206
#define PJX2_LXPOS				0x0210
#define PJX2_LYPOS				0x0211
#define PJX2_LTRG				0x0212
#define PJX2_RXPOS				0x0213
#define PJX2_RYPOS				0x0214
#define PJX2_RTRG				0x0215
#define PJX2_BTNS				0x0216
#define PJX3_LXPOS				0x0220
#define PJX3_LYPOS				0x0221
#define PJX3_LTRG				0x0222
#define PJX3_RXPOS				0x0223
#define PJX3_RYPOS				0x0224
#define PJX3_RTRG				0x0225
#define PJX3_BTNS				0x0226
#define PJX4_LXPOS				0x0230
#define PJX4_LYPOS				0x0231
#define PJX4_LTRG				0x0232
#define PJX4_RXPOS				0x0233
#define PJX4_RYPOS				0x0234
#define PJX4_RTRG				0x0235
#define PJX4_BTNS				0x0236
#define PJX_LXPOS				PJX1_LXPOS
#define PJX_LYPOS				PJX1_LYPOS
#define PJX_LTRG				PJX1_LTRG
#define PJX_RXPOS				PJX1_RXPOS
#define PJX_RYPOS				PJX1_RYPOS
#define PJX_RTRG				PJX1_RTRG
#define PJX_BTNS				PJX1_BTNS

#define MOUSE_RIGHT_BUTTON		0
#define MOUSE_LEFT_BUTTON		1
#define MOUSE_MIDDLE_BUTTON		2

class Input
{
private:
	Input() = default;
	~Input() = default;

	LPDIRECTINPUT8 lpDI = NULL;
	LPDIRECTINPUTDEVICE8 lpKeyboard = NULL;
	BYTE nowKeymap[256];
	BYTE frontKeymap[256];

	LPDIRECTINPUTDEVICE8 g_pDIMouse = NULL;	// �}�E�X�f�o�C�X
	DIMOUSESTATE nowMouseState;			// �}�E�X���
	DIMOUSESTATE frontMouseState;			// �}�E�X���
	COORD mousePos;

	XINPUT_STATE PreControllers[4];
	XINPUT_STATE NowControllers[4];

	static Input& GetInstance()
	{
		static Input instance;
		return instance;
	}

public:
	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;
	Input(Input&&) = delete;
	Input& operator=(Input&&) = delete;

	static void Setup(HWND hWnd, HINSTANCE hInstance)
	{
		Input& input = GetInstance();
		// IDirectInput8�̍쐬
		HRESULT ret = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&input.lpDI, NULL);
		if(FAILED(ret))
		{
			// �쐬�Ɏ��s
			MessageBox(NULL, "DirectInput8�̍쐬�Ɏ��s", "�G���[", MB_OK);
			return;
		}
		// IDirectInputDevice8�̎擾
		ret = input.lpDI->CreateDevice(GUID_SysKeyboard, &input.lpKeyboard, NULL);
		if(FAILED(ret))
		{
			MessageBox(NULL, "�L�[�{�[�h�f�o�C�X�̍쐬�Ɏ��s", "�G���[", MB_OK);
			input.lpDI->Release();
			return;
		}

		// ���̓f�[�^�`���̃Z�b�g
		ret = input.lpKeyboard->SetDataFormat(&c_dfDIKeyboard);
		if(FAILED(ret))
		{
			MessageBox(NULL, "���̓f�[�^�`���̃Z�b�g���s", "�G���[", MB_OK);
			input.lpKeyboard->Release();
			input.lpDI->Release();
			return;
		}
		// �r������̃Z�b�g
		ret = input.lpKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
		if(FAILED(ret))
		{
			MessageBox(NULL, "�r������̃Z�b�g���s", "�G���[", MB_OK);
			input.lpKeyboard->Release();
			input.lpDI->Release();
			return;
		}

		// �}�E�X�p�Ƀf�o�C�X�I�u�W�F�N�g���쐬
		ret = input.lpDI->CreateDevice(GUID_SysMouse, &input.g_pDIMouse, NULL);
		if(FAILED(ret))
		{
			MessageBox(NULL, "�}�E�X�f�o�C�X�̍쐬�Ɏ��s", "�G���[", MB_OK);
			input.lpDI->Release();
			return;
		}
		// �f�[�^�t�H�[�}�b�g��ݒ�
		ret = input.g_pDIMouse->SetDataFormat(&c_dfDIMouse);	// �}�E�X�p�̃f�[�^�E�t�H�[�}�b�g��ݒ�
		if(FAILED(ret))
		{
			MessageBox(NULL, "���̓f�[�^�`���̃Z�b�g���s", "�G���[", MB_OK);
			input.lpKeyboard->Release();
			input.lpDI->Release();
			// �f�[�^�t�H�[�}�b�g�Ɏ��s
			return;
		}
		// ���[�h��ݒ�i�t�H�A�O���E���h����r�����[�h�j
		ret = input.g_pDIMouse->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		if(FAILED(ret))
		{
			MessageBox(NULL, "�r������̃Z�b�g���s", "�G���[", MB_OK);
			input.lpKeyboard->Release();
			input.lpDI->Release();
			// ���[�h�̐ݒ�Ɏ��s
			return;
		}
		// �f�o�C�X�̐ݒ�
		DIPROPDWORD diprop;
		diprop.diph.dwSize = sizeof(diprop);
		diprop.diph.dwHeaderSize = sizeof(diprop.diph);
		diprop.diph.dwObj = 0;
		diprop.diph.dwHow = DIPH_DEVICE;
		diprop.dwData = DIPROPAXISMODE_REL;	// ���Βl���[�h�Őݒ�i��Βl��DIPROPAXISMODE_ABS�j

		ret = input.g_pDIMouse->SetProperty(DIPROP_AXISMODE, &diprop.diph);
		if(FAILED(ret))
		{
			// �f�o�C�X�̐ݒ�Ɏ��s
			return;
		}
		// ���͐���J�n
		input.g_pDIMouse->Acquire();
	}

	static void Finalize()
	{
		GetInstance().lpKeyboard->Release();
		GetInstance().lpDI->Release();
		GetInstance().g_pDIMouse->Release();
	}

	static void Update()
	{
		Input& input = GetInstance();
		memcpy(input.frontKeymap, input.nowKeymap, sizeof(input.nowKeymap));
		input.frontMouseState = input.nowMouseState;

		//X�C���v�b�g(�R���g���[��)
		memcpy_s(input.PreControllers, sizeof(XINPUT_STATE) * 4, input.NowControllers, sizeof(XINPUT_STATE) * 4);
		for(int i = 0; i < 4; i++)
		{
			XInputGetState(i, &input.NowControllers[i]);
		}

		// �L�[�̓���
		ZeroMemory(input.nowKeymap, sizeof(input.nowKeymap));
		HRESULT ret = input.lpKeyboard->GetDeviceState(sizeof(input.nowKeymap), input.nowKeymap);
		if(FAILED(ret))
		{
			// ���s�Ȃ�ĊJ�����Ă�����x�擾
			input.lpKeyboard->Acquire();
			input.lpKeyboard->GetDeviceState(sizeof(input.nowKeymap), input.nowKeymap);
		}

		HRESULT hr = input.g_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &input.nowMouseState);
		if(FAILED(hr))
		{
			input.g_pDIMouse->Acquire();
			hr = input.g_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &input.nowMouseState);
		}

		input.mousePos.X += GetMouseMovePos().X;
		input.mousePos.Y += GetMouseMovePos().Y;

		if(GetKeyDown(DIK_ESCAPE))
			ClipCursor(NULL);
	}

	static bool GetKey(int key)
	{
		return GetInstance().nowKeymap[key] & 0x80;
	}

	static bool GetKeyDown(int key)
	{
		return (GetInstance().nowKeymap[key] & 0x80) & (~(GetInstance().frontKeymap[key] & 0x80));
	}

	static bool GetKeyUp(int key)
	{
		return (~(GetInstance().nowKeymap[key] & 0x80)) & (GetInstance().frontKeymap[key] & 0x80);
	}

	static bool GetMouseButton(int button)
	{
		return GetInstance().nowMouseState.rgbButtons[button] & 0x80;
	}

	static bool GetMouseButtonDown(int button)
	{
		return (~(GetInstance().nowMouseState.rgbButtons[button] & 0x80)) & (GetInstance().frontMouseState.rgbButtons[
			button] & 0x80);
	}

	static COORD GetMousePos()
	{
		return GetInstance().mousePos;
	}

	static COORD GetMouseMovePos()
	{
		COORD pos;
		pos.X = static_cast<SHORT>(GetInstance().frontMouseState.lX * 2);
		pos.Y = static_cast<SHORT>(GetInstance().frontMouseState.lY * 2);
		return pos;
	}

	static int GetMoveMouseWheel()
	{
		return static_cast<int>(GetInstance().nowMouseState.lZ);
	}

	static bool GetButton(int port, int button)
	{
		int id, func;
		if((port & 0xfe00) == 0x0200)
		{
			id = (port & 0x01f0) >> 4;
			func = port & 0x0f;

			switch(func)
			{
			case 0:
				return (GetInstance().NowControllers[id].Gamepad.sThumbLX);
			case 1:
				return (GetInstance().NowControllers[id].Gamepad.sThumbLY);
			case 2:
				return (GetInstance().NowControllers[id].Gamepad.bLeftTrigger);
			case 3:
				return (GetInstance().NowControllers[id].Gamepad.sThumbRX);
			case 4:
				return (GetInstance().NowControllers[id].Gamepad.sThumbRY);
			case 5:
				return (GetInstance().NowControllers[id].Gamepad.bRightTrigger);
			case 6:
				return (GetInstance().NowControllers[id].Gamepad.wButtons & button);
			}
		}
	}

	//���ƑO���r���đO��������ĂȂ��č�������Ă�Ƃ�(�R���g���[���[)
	static bool GetButtonDown(int port, int button)
	{
		Input& input = GetInstance();

		if((port & 0xfe00) == 0x0200)  //�R���g���[���[�̏ꍇ
		{
			int id, func;
			bool now, pre;

			id = (port & 0x01f0) >> 4;
			func = port & 0x0f;

			switch(func)
			{
			case 0:
				now = (input.NowControllers[id].Gamepad.sThumbLX) + (input.NowControllers[id].Gamepad.sThumbLY);
				pre = (input.PreControllers[id].Gamepad.sThumbLX) + (input.PreControllers[id].Gamepad.sThumbLY);
				break;
			case 1:
				now = (input.NowControllers[id].Gamepad.sThumbLY) + (input.NowControllers[id].Gamepad.sThumbLX);
				pre = (input.PreControllers[id].Gamepad.sThumbLY) + (input.PreControllers[id].Gamepad.sThumbLX);
				break;
			case 2:
				now = (input.NowControllers[id].Gamepad.bLeftTrigger == button);
				pre = (input.PreControllers[id].Gamepad.bLeftTrigger == button);
				break;
			case 3:
				now = (input.NowControllers[id].Gamepad.sThumbRX) + (input.NowControllers[id].Gamepad.sThumbRY);
				pre = (input.PreControllers[id].Gamepad.sThumbRX) + (input.PreControllers[id].Gamepad.sThumbRY);
				break;
			case 4:
				now = (input.NowControllers[id].Gamepad.sThumbRY) + (input.NowControllers[id].Gamepad.sThumbRX);
				pre = (input.PreControllers[id].Gamepad.sThumbRY) + (input.PreControllers[id].Gamepad.sThumbRX);
				break;
			case 5:
				now = (input.NowControllers[id].Gamepad.bRightTrigger == button);
				pre = (input.PreControllers[id].Gamepad.bRightTrigger == button);
				break;
			case 6:
				now = (input.NowControllers[id].Gamepad.wButtons == button);
				pre = (input.PreControllers[id].Gamepad.wButtons == button);
				break;
			}
			return now & (!pre);
		}
		return false;
	}

	//���ƑO���r���đO��������Ăč�������ĂȂ��Ƃ�(�R���g���[���[)
	static bool GetButtonUp(int port, int button)
	{
		Input& input = GetInstance();

		if((port & 0xfe00) == 0x0200)
		{
			int id, func;
			bool now, pre;

			id = (port & 0x01f0) >> 4;
			func = port & 0x0f;

			switch(func)
			{
			case 0:
				now = (input.NowControllers[id].Gamepad.sThumbLX) + (input.NowControllers[id].Gamepad.sThumbLY);
				pre = (input.PreControllers[id].Gamepad.sThumbLX) + (input.PreControllers[id].Gamepad.sThumbLY);
				break;
			case 1:
				now = (input.NowControllers[id].Gamepad.sThumbLY) + (input.NowControllers[id].Gamepad.sThumbLX);
				pre = (input.PreControllers[id].Gamepad.sThumbLY) + (input.PreControllers[id].Gamepad.sThumbLX);
				break;
			case 2:
				now = (input.NowControllers[id].Gamepad.bLeftTrigger == button);
				pre = (input.PreControllers[id].Gamepad.bLeftTrigger == button);
				break;
			case 3:
				now = (input.NowControllers[id].Gamepad.sThumbRX) + (input.NowControllers[id].Gamepad.sThumbRY);
				pre = (input.PreControllers[id].Gamepad.sThumbRX) + (input.PreControllers[id].Gamepad.sThumbRY);
				break;
			case 4:
				now = (input.NowControllers[id].Gamepad.sThumbRY) + (input.NowControllers[id].Gamepad.sThumbRX);
				pre = (input.PreControllers[id].Gamepad.sThumbRY) + (input.PreControllers[id].Gamepad.sThumbRX);
				break;
			case 5:
				now = (input.NowControllers[id].Gamepad.bRightTrigger == button);
				pre = (input.PreControllers[id].Gamepad.bRightTrigger == button);
				break;
			case 6:
				now = (input.NowControllers[id].Gamepad.wButtons == button);
				pre = (input.PreControllers[id].Gamepad.wButtons == button);
				break;
			}
			return (!now) & pre;
		}
		return false;
	}

	static COORD GetXLeftPos()
	{
		COORD ret;
		ret.X = GetInstance().NowControllers[(PJX_LXPOS & 0x01f0) >> 4].Gamepad.sThumbLX;
		ret.Y = GetInstance().NowControllers[(PJX_LYPOS & 0x01f0) >> 4].Gamepad.sThumbLY;
		return ret;
	}

	static COORD GetXRightPos()
	{
		COORD ret;
		ret.X = GetInstance().NowControllers[(PJX_RXPOS & 0x01f0) >> 4].Gamepad.sThumbRX;
		ret.Y = GetInstance().NowControllers[(PJX_RYPOS & 0x01f0) >> 4].Gamepad.sThumbRY;
		return ret;
	}

	static void CursorInvisible(HWND hWnd)
	{
		//�J�[�\���ړ��ʒu�𐧌�
		RECT windowRect, rect;
		GetWindowRect(hWnd, &windowRect);
		rect.top = windowRect.top - (windowRect.top - windowRect.bottom) / 2;
		rect.bottom = rect.top;
		rect.right = windowRect.right - (windowRect.right - windowRect.left) / 2;
		rect.left = rect.right;
		ClipCursor(&rect);
		ShowCursor(FALSE);
	}
};
