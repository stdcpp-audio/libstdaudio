// libstdaudio
// Copyright (c) 2019 - Guy Somberg
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

#pragma once

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
	static const CLSID CLSID_MMDeviceEnumerator;
	static const IID IID_IMMDeviceEnumerator;
	static const IID IID_IAudioClient;
	static const IID IID_IAudioRenderClient;
	static const IID IID_IAudioCaptureClient;

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

const CLSID __wasapi_util::CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID __wasapi_util::IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID __wasapi_util::IID_IAudioClient = __uuidof(IAudioClient);
const IID __wasapi_util::IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID __wasapi_util::IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

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

	audio_device& operator=(audio_device&& other)
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
		return _pAudioCaptureClient != nullptr;
	}

	bool is_output() const noexcept
	{
		return _pAudioRenderClient != nullptr;
	}

	int get_num_input_channels() const noexcept
	{
		return is_input() ? _MixFormat->nChannels : 0;
	}

	int get_num_output_channels() const noexcept
	{
		return is_output() ? _MixFormat->nChannels : 0;
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
		return _MixFormat->nSamplesPerSec;
	}

	span<const sample_rate_t> get_supported_sample_rates() const noexcept
	{
		return {};
	}

	bool set_sample_rate(sample_rate_t new_sample_rate)
	{
		_MixFormat->nSamplesPerSec = new_sample_rate;
	}

	using buffer_size_t = WORD;

	buffer_size_t get_buffer_size_frames() const noexcept
	{
		return _MixFormat->nBlockAlign;
	}

	span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept
	{
		return {};
	}

	bool set_buffer_size_frames(buffer_size_t new_buffer_size)
	{
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

			hr = _pAudioClient->SetEventHandle(_hEvent);
			if (FAILED(hr))
				return false;

			hr = _pAudioClient->Start();
			if (FAILED(hr))
				return false;

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
			_running = true;
		}

		return true;
	}

	bool stop()
	{
		if (_running)
		{
			_running = false;
			_ProcessingThread.join();

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
		if (is_output())
		{
			UINT32 CurrentPadding = 0;
			_pAudioClient->GetCurrentPadding(&CurrentPadding);
			if (CurrentPadding == 0)
				return;

			BYTE* pData = nullptr;
			_pAudioRenderClient->GetBuffer(CurrentPadding, &pData);
			if (pData == nullptr)
				return;

			audio_device_io<__wasapi_native_sample_type> device_io;
			device_io.output_buffer = {reinterpret_cast<__wasapi_native_sample_type*>(pData), CurrentPadding, _MixFormat->nChannels, contiguous_interleaved };
			callback(*this, device_io);

			_pAudioRenderClient->ReleaseBuffer(CurrentPadding, 0);
		}
		else if (is_input())
		{
			UINT32 CurrentPadding = 0;
			_pAudioClient->GetCurrentPadding(&CurrentPadding);
			if (CurrentPadding == 0)
				return;

			BYTE* pData = nullptr;
			_pAudioCaptureClient->GetBuffer(CurrentPadding, &pData);
			if (pData == nullptr)
				return;

			audio_device_io<__wasapi_native_sample_type> device_io;
			device_io.input_buffer = { reinterpret_cast<__wasapi_native_sample_type*>(pData), CurrentPadding, _MixFormat->nChannels, contiguous_interleaved };
			callback(*this, device_io);

			_pAudioCaptureClient->ReleaseBuffer(CurrentPadding, 0);
		}
	}

	bool has_unprocessed_io() const noexcept
	{
		UINT32 CurrentPadding = 0;
		_pAudioClient->GetCurrentPadding(&CurrentPadding);
		return CurrentPadding > 0;
	}

private:
	friend class __audio_device_enumerator;

	audio_device(IMMDevice* pDevice) :
		_pDevice(pDevice)
	{
		assert(_pDevice != nullptr);

		_init_device_id_and_name();
		assert(!_device_id.empty());
		assert(!_name.empty());

		_init_audio_client();
		assert(_pAudioClient != nullptr);
		assert(_pAudioCaptureClient != nullptr || _pAudioRenderClient != nullptr);

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
		HRESULT hr = _pDevice->Activate(__wasapi_util::IID_IAudioClient, CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&_pAudioClient));
		if (FAILED(hr))
			return;

		/*HRESULT render_hr =*/ _pAudioClient->GetService(__wasapi_util::IID_IAudioRenderClient, reinterpret_cast<void**>(&_pAudioRenderClient));
		/*HRESULT capture_hr =*/ _pAudioClient->GetService(__wasapi_util::IID_IAudioCaptureClient, reinterpret_cast<void**>(&_pAudioCaptureClient));
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
	WAVEFORMATEX* _MixFormat;
	thread _ProcessingThread;

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
		return get_device_list([](const audio_device& d)
		{
			return d.is_input();
		});
	}

	static auto get_output_device_list()
	{
		return get_device_list([](const audio_device& d)
		{
			return d.is_output();
		});
	}

private:
	__audio_device_enumerator() = delete;

	static optional<audio_device> get_default_device(bool bOutputDevice)
	{
		__wasapi_util::CCoInitialize ComInitializer;

		IMMDeviceEnumerator* pEnumerator = nullptr;
		__wasapi_util::AutoRelease EnumeratorRelease{ pEnumerator };

		HRESULT hr = CoCreateInstance(
			__wasapi_util::CLSID_MMDeviceEnumerator, nullptr,
			CLSCTX_ALL, __wasapi_util::IID_IMMDeviceEnumerator,
			reinterpret_cast<void**>(&pEnumerator));

		if (FAILED(hr))
			return nullopt;

		IMMDevice* pDevice = nullptr;
		hr = pEnumerator->GetDefaultAudioEndpoint(bOutputDevice ? eRender : eCapture, eConsole, &pDevice);
		if (FAILED(hr))
			return nullopt;

		return audio_device{ pDevice };
	}

	static vector<IMMDevice*> get_devices()
	{
		__wasapi_util::CCoInitialize ComInitializer;

		IMMDeviceEnumerator* pEnumerator = nullptr;
		__wasapi_util::AutoRelease EnumeratorRelease{ pEnumerator };
		HRESULT hr = CoCreateInstance(
			__wasapi_util::CLSID_MMDeviceEnumerator, nullptr,
			CLSCTX_ALL, __wasapi_util::IID_IMMDeviceEnumerator,
			reinterpret_cast<void**>(&pEnumerator));
		if (FAILED(hr))
			return {};

		IMMDeviceCollection* pDeviceCollection = nullptr;
		__wasapi_util::AutoRelease CollectionRelease{ pDeviceCollection };

		hr = pEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATEMASK_ALL, &pDeviceCollection);
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

	template <typename Condition>
	static auto get_device_list(Condition condition)
	{
		audio_device_list devices;
		const auto mmdevices = get_devices();

		for (auto* mmdevice : mmdevices) {
			auto device = audio_device{ mmdevice };
			if (condition(device))
				devices.push_front(move(device));
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

_LIBSTDAUDIO_NAMESPACE_END
