// libstdaudio
// Copyright (c) 2019 - Guy Somberg
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

#define NOMINMAX
#include <cctype>
#include <codecvt>
#include <string>
#include <iostream>
#include <vector>
#include <functional>
#include <thread>
#include <forward_list>
#include <atomic>
#include <cassert>
#include <string_view>
#include <initguid.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <Functiondiscoverykeys_devpkey.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: make __wasapi_sample_type flexible according to the recommendation (see AudioSampleType).
using __wasapi_native_sample_type = float;

class __wasapi_util
{
public:
	static const CLSID& GetMMDeviceEnumeratorClassId()
	{
		static const CLSID MMDeviceEnumeratorClassId = __uuidof(MMDeviceEnumerator);
		return MMDeviceEnumeratorClassId;
	}
	static const IID& GetIMMDeviceEnumeratorInterfaceId()
	{
		static const IID IMMDeviceEnumeratorInterfaceId = __uuidof(IMMDeviceEnumerator);
		return IMMDeviceEnumeratorInterfaceId;
	}
	static const IID& GetIAudioClientInterfaceId()
	{
		static const IID IAudioClientInterfaceId = __uuidof(IAudioClient);
		return IAudioClientInterfaceId;
	}
	static const IID& GetIAudioRenderClientInterfaceId()
	{
		static const IID IAudioRenderClientInterfaceId = __uuidof(IAudioRenderClient);
		return IAudioRenderClientInterfaceId;
	}
	static const IID& GetIAudioCaptureClientInterfaceId()
	{
		static const IID IAudioCaptureClientInterfaceId = __uuidof(IAudioCaptureClient);
		return IAudioCaptureClientInterfaceId;
	}

	class CCoInitialize
	{
	public:
		CCoInitialize() : m_hr(CoInitialize(nullptr)) { }
		~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
		operator HRESULT() const { return m_hr; }
		HRESULT m_hr;
	};

	template<typename T>
	class AutoRelease
	{
	public:
		AutoRelease(T*& pValue) :
			_pValue(pValue)
		{}

		~AutoRelease()
		{
			if (_pValue != nullptr)
				_pValue->Release();
		}

	private:
		T*& _pValue;
	};

	static string ConvertString(const wchar_t* widestring)
	{
		int RequiredCharacters = WideCharToMultiByte(CP_UTF8, 0, widestring, -1, nullptr, 0, nullptr, nullptr);
		if (RequiredCharacters <= 0)
			return {};

		string Output;
		Output.resize(static_cast<size_t>(RequiredCharacters));
		WideCharToMultiByte(CP_UTF8, 0, widestring, -1, Output.data(), static_cast<int>(Output.size()), nullptr, nullptr);
		return Output;
	}

	static string ConvertString(const wstring& input)
	{
		int RequiredCharacters = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);
		if (RequiredCharacters <= 0)
			return {};

		string Output;
		Output.resize(static_cast<size_t>(RequiredCharacters));
		WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), Output.data(), static_cast<int>(Output.size()), nullptr, nullptr);
		return Output;
	}
};

struct audio_device_exception : public runtime_error
{
	explicit audio_device_exception(const char* what)
		: runtime_error(what)
	{
	}
};

class audio_device
{
public:
	audio_device() = delete;
	audio_device(const audio_device&) = delete;
	audio_device& operator=(const audio_device&) = delete;

	audio_device(audio_device&& other) :
		_pDevice(other._pDevice),
		_pAudioClient(other._pAudioClient),
		_pAudioCaptureClient(other._pAudioCaptureClient),
		_pAudioRenderClient(other._pAudioRenderClient),
		_hEvent(other._hEvent),
		_device_id(std::move(other._device_id)),
		_running(other._running.load()),
		_name(std::move(other._name)),
		_MixFormat(other._MixFormat),
		_ProcessingThread(std::move(other._ProcessingThread)),
		_bufferFrameCount(other._bufferFrameCount),
		_IsRenderDevice(other._IsRenderDevice),
		_stop_callback(std::move(other._stop_callback)),
		_user_callback(std::move(other._user_callback))
	{
		other._pDevice = nullptr;
		other._pAudioClient = nullptr;
		other._pAudioCaptureClient = nullptr;
		other._pAudioRenderClient = nullptr;
		other._hEvent = nullptr;
		other._MixFormat = nullptr;
	}

	audio_device& operator=(audio_device&& other) noexcept
	{
		if (this == &other)
			return *this;

		_pDevice = other._pDevice;
		_pAudioClient = other._pAudioClient;
		_pAudioCaptureClient = other._pAudioCaptureClient;
		_pAudioRenderClient = other._pAudioRenderClient;
		_hEvent = other._hEvent;
		_device_id = std::move(other._device_id);
		_running = other._running.load();
		_name = std::move(other._name);
		_MixFormat = other._MixFormat;
		_ProcessingThread = std::move(other._ProcessingThread);
		_bufferFrameCount = other._bufferFrameCount;
		_IsRenderDevice = other._IsRenderDevice;
		_stop_callback = std::move(other._stop_callback);
		_user_callback = std::move(other._user_callback);

		other._pDevice = nullptr;
		other._pAudioClient = nullptr;
		other._pAudioCaptureClient = nullptr;
		other._pAudioRenderClient = nullptr;
		other._hEvent = nullptr;
		other._MixFormat = nullptr;
	}

	~audio_device()
	{
		stop();

		if (_MixFormat != nullptr)
			CoTaskMemFree(_MixFormat);

		if (_pAudioCaptureClient != nullptr)
			_pAudioCaptureClient->Release();

		if (_pAudioRenderClient != nullptr)
			_pAudioRenderClient->Release();

		if (_pAudioClient != nullptr)
			_pAudioClient->Release();

		if (_pDevice != nullptr)
			_pDevice->Release();
	}

	string_view name() const noexcept
	{
		return _name;
	}

	using device_id_t = wstring;

	device_id_t device_id() const noexcept
	{
		return _device_id;
	}

	bool is_input() const noexcept
	{
		return _IsRenderDevice == false;
	}

	bool is_output() const noexcept
	{
		return _IsRenderDevice == true;
	}

	int get_num_input_channels() const noexcept
	{
		if (is_input() == false)
			return 0;

		return (_MixFormat != nullptr) ? _MixFormat->nChannels : 0;
	}

	int get_num_output_channels() const noexcept
	{
		if (is_output() == false)
			return 0;

		return (_MixFormat != nullptr) ? _MixFormat->nChannels : 0;
	}

	using sample_rate_t = DWORD;

	// TODO: WASAPI allows me to ask whether or not a particular mix format is valid,
	// but does not provide a way to enumerate valid formats.
	// Code on the Internet seems to indicate that if you drop down to waveOut/waveIn,
	// then you can query the device caps using waveOutGetDevCaps()/waveInGetDevCaps().
	// That will give you a bit-field containing supported commonly-used settings.
	// That seems like an awful lot of work, but maybe it's okay because we can just
	// do it in the constructor and call it done.
	// Reference on how to match WASAPI to WaveOut:
	// https://docs.microsoft.com/en-us/windows/desktop/CoreAudio/device-roles-for-legacy-windows-multimedia-applications
	sample_rate_t get_sample_rate() const noexcept
	{
		if (_MixFormat == nullptr)
			return 0;

		return _MixFormat->nSamplesPerSec;
	}

	span<const sample_rate_t> get_supported_sample_rates() const noexcept
	{
		return {};
	}

	bool set_sample_rate(sample_rate_t new_sample_rate)
	{
		if (_MixFormat == nullptr)
			return false;

		_MixFormat->nSamplesPerSec = new_sample_rate;
		return true;
	}

	using buffer_size_t = WORD;

	buffer_size_t get_buffer_size_frames() const noexcept
	{
		if (_MixFormat == nullptr)
			return 0;

		return _MixFormat->nBlockAlign;
	}

	span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept
	{
		return {};
	}

	bool set_buffer_size_frames(buffer_size_t new_buffer_size)
	{
		if (_MixFormat == nullptr)
			return false;

		_MixFormat->nBlockAlign = new_buffer_size;
		return true;
	}

	template <typename _SampleType>
	constexpr bool supports_sample_type() const noexcept
	{
		// TODO: How do we support different sample types?
		return is_same_v<_SampleType, __wasapi_native_sample_type>;
	}

	constexpr bool can_connect() const noexcept
	{
		return true;
	}

	constexpr bool can_process() const noexcept
	{
		return true;
	}

	template <typename _CallbackType,
		typename = enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<__wasapi_native_sample_type>&>>>
	void connect(_CallbackType callback)
	{
		if (_running)
			throw audio_device_exception("cannot connect to running audio_device");

		_user_callback = move(callback);
	}

	bool start()
	{
		static auto no_op = [](audio_device&) noexcept {};
		return start(no_op, no_op);
	}

	template <typename _StartCallbackType, typename _StopCallbackType,
		typename = enable_if_t<is_nothrow_invocable_v<_StartCallbackType, audio_device&> && is_nothrow_invocable_v<_StopCallbackType, audio_device&>>>
		bool start(_StartCallbackType&& start_callback, _StopCallbackType&& stop_callback)
	{
		if (_pAudioClient == nullptr)
			return false;

		if (!_running)
		{
			_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (_hEvent == nullptr)
				return false;

			REFERENCE_TIME periodicity = 0;
			REFERENCE_TIME buffer_duration = 10000000;
			HRESULT hr = _pAudioClient->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
				buffer_duration,
				periodicity,
				_MixFormat,
				nullptr);

			// TODO: Deal with AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED return code by resetting the buffer_duration and retrying:
			// https://docs.microsoft.com/en-us/windows/desktop/api/audioclient/nf-audioclient-iaudioclient-initialize
			if (FAILED(hr))
				return false;

			/*HRESULT render_hr =*/ _pAudioClient->GetService(__wasapi_util::GetIAudioRenderClientInterfaceId(), reinterpret_cast<void**>(&_pAudioRenderClient));
			/*HRESULT capture_hr =*/ _pAudioClient->GetService(__wasapi_util::GetIAudioCaptureClientInterfaceId(), reinterpret_cast<void**>(&_pAudioCaptureClient));

			// TODO: Make sure to clean up more gracefully from errors
			hr = _pAudioClient->GetBufferSize(&_bufferFrameCount);
			if (FAILED(hr))
				return false;

			hr = _pAudioClient->SetEventHandle(_hEvent);
			if (FAILED(hr))
				return false;

			hr = _pAudioClient->Start();
			if (FAILED(hr))
				return false;

			_running = true;

			if (_user_callback)
			{
				_ProcessingThread = thread{ [this]()
				{
					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

					while (_running)
					{
						wait();
						process(_user_callback);
					}
				} };
			}

			start_callback(*this);
			_stop_callback = stop_callback;
		}

		return true;
	}

	bool stop()
	{
		if (_running)
		{
			_running = false;

			if (_ProcessingThread.joinable())
				_ProcessingThread.join();

			if (_pAudioClient != nullptr)
				_pAudioClient->Stop();
			if (_hEvent != nullptr)
			{
				CloseHandle(_hEvent);
			}
			_stop_callback(*this);
		}

		return true;
	}

	bool is_running() const noexcept
	{
		return _running;
	}

	void wait() const
	{
		WaitForSingleObject(_hEvent, INFINITE);
	}

	template <typename _CallbackType,
		typename = enable_if_t<is_invocable_v<_CallbackType, audio_device&, audio_device_io<__wasapi_native_sample_type>&>>>
	void process(_CallbackType& callback)
	{
		if (_pAudioClient == nullptr
			|| _MixFormat == nullptr)
			return;

		if (is_output())
		{
			UINT32 CurrentPadding = 0;
			_pAudioClient->GetCurrentPadding(&CurrentPadding);

			auto NumFramesAvailable = _bufferFrameCount - CurrentPadding;
			if (NumFramesAvailable == 0)
				return;

			BYTE* pData = nullptr;
			_pAudioRenderClient->GetBuffer(NumFramesAvailable, &pData);
			if (pData == nullptr)
				return;

			audio_device_io<__wasapi_native_sample_type> device_io;
			device_io.output_buffer = {reinterpret_cast<__wasapi_native_sample_type*>(pData), NumFramesAvailable, _MixFormat->nChannels, contiguous_interleaved };
			callback(*this, device_io);

			_pAudioRenderClient->ReleaseBuffer(NumFramesAvailable, 0);
		}
		else if (is_input())
		{
			UINT32 NumFrames = 0;
			_pAudioCaptureClient->GetNextPacketSize(&NumFrames);
			if (NumFrames == 0)
				return;

			// TODO: Support device position.
			DWORD dwFlags = 0;
			BYTE* pData = nullptr;
			_pAudioCaptureClient->GetBuffer(&pData, &NumFrames, &dwFlags, nullptr, nullptr);
			if (pData == nullptr)
				return;

			audio_device_io<__wasapi_native_sample_type> device_io;
			device_io.input_buffer = { reinterpret_cast<__wasapi_native_sample_type*>(pData), NumFrames, _MixFormat->nChannels, contiguous_interleaved };
			callback(*this, device_io);

			_pAudioCaptureClient->ReleaseBuffer(NumFrames);
		}
	}

	bool has_unprocessed_io() const noexcept
	{
		if (_pAudioClient == nullptr)
			return false;

		UINT32 CurrentPadding = 0;
		_pAudioClient->GetCurrentPadding(&CurrentPadding);
		return CurrentPadding > 0;
	}

private:
	friend class __audio_device_enumerator;

	audio_device(IMMDevice* pDevice, bool bIsRenderDevice) :
		_pDevice(pDevice),
		_IsRenderDevice(bIsRenderDevice)
	{
		assert(_pDevice != nullptr);

		_init_device_id_and_name();
		assert(!_device_id.empty());
		assert(!_name.empty());

		_init_audio_client();
		if (_pAudioClient == nullptr)
			return;

		_init_mix_format();
		assert(_MixFormat != nullptr);
	}

	void _init_device_id_and_name()
	{
		LPWSTR DeviceId = nullptr;
		HRESULT hr = _pDevice->GetId(&DeviceId);
		if (SUCCEEDED(hr))
		{
			_device_id = DeviceId;
			CoTaskMemFree(DeviceId);
		}

		IPropertyStore* pPropertyStore = nullptr;
		__wasapi_util::AutoRelease AutoReleasePropertyStore{ pPropertyStore };

		hr = _pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
		if (SUCCEEDED(hr))
		{
			PROPVARIANT PropertyVariant;
			PropVariantInit(&PropertyVariant);

			hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &PropertyVariant);
			if (SUCCEEDED(hr))
			{
				_name = __wasapi_util::ConvertString(PropertyVariant.pwszVal);
			}

			PropVariantClear(&PropertyVariant);
		}
	}

	void _init_audio_client()
	{
		HRESULT hr = _pDevice->Activate(__wasapi_util::GetIAudioClientInterfaceId(), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&_pAudioClient));
		if (FAILED(hr))
			return;
	}

	void _init_mix_format()
	{
		/*HRESULT hr =*/ _pAudioClient->GetMixFormat(&_MixFormat);
	}

	IMMDevice* _pDevice = nullptr;
	IAudioClient* _pAudioClient = nullptr;
	IAudioCaptureClient* _pAudioCaptureClient = nullptr;
	IAudioRenderClient* _pAudioRenderClient = nullptr;
	HANDLE _hEvent;
	wstring _device_id;
	atomic<bool> _running = false;
	string _name;

	// TODO: Make this a WAVEFORMATEXTENSIBLE value type, then copy the mix format into this
	// so that we have fewer error (and edge) cases.
	WAVEFORMATEX* _MixFormat = nullptr;
	thread _ProcessingThread;
	UINT32 _bufferFrameCount = 0;
	bool _IsRenderDevice = true;

	using __stop_callback_t = function<void(audio_device&)>;
	__stop_callback_t _stop_callback;
	using __wasapi_callback_t = function<void(audio_device&, audio_device_io<__wasapi_native_sample_type>&)>;
	__wasapi_callback_t _user_callback;
	audio_device_io<__wasapi_native_sample_type> _current_buffers;
	__wasapi_util::CCoInitialize ComInitializer;
};

class audio_device_list : public forward_list<audio_device> {
};

class __audio_device_enumerator {
public:
	static optional<audio_device> get_default_output_device()
	{
		const bool bIsOutputDevice = true;
		return get_default_device(bIsOutputDevice);
	};

	static optional<audio_device> get_default_input_device()
	{
		const bool bIsOutputDevice = false;
		return get_default_device(bIsOutputDevice);
	};

	static auto get_input_device_list()
	{
		return get_device_list(false);
	}

	static auto get_output_device_list()
	{
		return get_device_list(true);
	}

private:
	__audio_device_enumerator() = delete;

	static optional<audio_device> get_default_device(bool bOutputDevice)
	{
		__wasapi_util::CCoInitialize ComInitializer;

		IMMDeviceEnumerator* pEnumerator = nullptr;
		__wasapi_util::AutoRelease EnumeratorRelease{ pEnumerator };

		HRESULT hr = CoCreateInstance(
			__wasapi_util::GetMMDeviceEnumeratorClassId(), nullptr,
			CLSCTX_ALL, __wasapi_util::GetIMMDeviceEnumeratorInterfaceId(),
			reinterpret_cast<void**>(&pEnumerator));

		if (FAILED(hr))
			return nullopt;

		IMMDevice* pDevice = nullptr;
		hr = pEnumerator->GetDefaultAudioEndpoint(bOutputDevice ? eRender : eCapture, eConsole, &pDevice);
		if (FAILED(hr))
			return nullopt;

		return audio_device{ pDevice, bOutputDevice };
	}

	static vector<IMMDevice*> get_devices(bool bOutputDevices)
	{
		__wasapi_util::CCoInitialize ComInitializer;

		IMMDeviceEnumerator* pEnumerator = nullptr;
		__wasapi_util::AutoRelease EnumeratorRelease{ pEnumerator };
		HRESULT hr = CoCreateInstance(
			__wasapi_util::GetMMDeviceEnumeratorClassId(), nullptr,
			CLSCTX_ALL, __wasapi_util::GetIMMDeviceEnumeratorInterfaceId(),
			reinterpret_cast<void**>(&pEnumerator));
		if (FAILED(hr))
			return {};

		IMMDeviceCollection* pDeviceCollection = nullptr;
		__wasapi_util::AutoRelease CollectionRelease{ pDeviceCollection };

		EDataFlow SelectedDataFlow = bOutputDevices ? eRender : eCapture;
		hr = pEnumerator->EnumAudioEndpoints(SelectedDataFlow, DEVICE_STATEMASK_ALL, &pDeviceCollection);
		if (FAILED(hr))
			return {};

		UINT DeviceCount = 0;
		hr = pDeviceCollection->GetCount(&DeviceCount);
		if (FAILED(hr))
			return {};

		vector<IMMDevice*> Devices;
		for (UINT i = 0; i < DeviceCount; i++)
		{
			IMMDevice* pDevice = nullptr;
			hr = pDeviceCollection->Item(i, &pDevice);
			if (FAILED(hr))
			{
				if (pDevice != nullptr)
				{
					pDevice->Release();
				}
				continue;
			}

			if (pDevice != nullptr)
				Devices.push_back(pDevice);
		}

		return Devices;
	}

	static audio_device_list get_device_list(bool bOutputDevices)
	{
		__wasapi_util::CCoInitialize ComInitializer;
		audio_device_list devices;
		const auto mmdevices = get_devices(bOutputDevices);

		for (auto* mmdevice : mmdevices) {
			devices.push_front(audio_device{ mmdevice, bOutputDevices });
		}

		return devices;
	}
};

optional<audio_device> get_default_audio_input_device()
{
	return __audio_device_enumerator::get_default_input_device();
}

optional<audio_device> get_default_audio_output_device()
{
	return __audio_device_enumerator::get_default_output_device();
}

audio_device_list get_audio_input_device_list()
{
	return __audio_device_enumerator::get_input_device_list();
}

audio_device_list get_audio_output_device_list()
{
	return __audio_device_enumerator::get_output_device_list();
}

template <typename F, typename /*= enable_if_t<std::is_invocable_v<F>>*/>
void set_audio_device_list_callback(audio_device_list_event, F&&)
{

}
_LIBSTDAUDIO_NAMESPACE_END
