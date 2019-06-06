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
#include <forward_list>
#include <initguid.h>
#include <audioclient.h>
#include <mmdeviceapi.h>

_LIBSTDAUDIO_NAMESPACE_BEGIN

// TODO: make __wasapi_sample_type flexible according to the recommendation (see AudioSampleType).
using __wasapi_native_sample_type = float;

class __wasapi_util
{
public:
	static const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
	static const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
	static const IID IID_IAudioClient = __uuidof(IAudioClient);
	static const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
	static const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

	class CCoInitialize
	{
	public:
		CCoInitialize() : m_hr(CoInitialize(nullptr)) { }
		~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
		operator HRESULT() const { return m_hr; }
		HRESULT m_hr;
	};

	class AutoRelease
	{
	public:
		AutoRelease(IUnknown*& pUnk) :
			_pUnk(pUnk)
		{}

		~AutoRelease()
		{
			if (_pUnk != nullptr)
				_pUnk->Release();
		}

	private:
		IUnknown*& _pUnk;
	};
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

	sample_rate_t get_sample_rate() const noexcept
	{
		return _MixFormat->nSamplesPerSec;
	}

	span<const sample_rate_t> get_supported_sample_rates() const noexcept
	{
	}

	bool set_sample_rate(sample_rate_t new_sample_rate)
	{
		_MixFormat->nSamplesPerSec = new_sample_rate;
	}

	using buffer_size_t = uint32_t;

	// TODO: WASAPI appears to support arbitrary-sized buffers.
	// How does this API reflect that?
	// Oh, wait, maybe it's coming from WaveFormat.nBlockAlign?
	// Then how do I get the available allowed buffer sizes?
	// And what is the relationship of this to hnsBufferDuration
	// that is passed to AudioClient::Initialize()?
	buffer_size_t get_buffer_size_frames() const noexcept
	{
		return 0;
	}

	span<const buffer_size_t> get_supported_buffer_sizes_frames() const noexcept
	{
		return {};
	}

	bool set_buffer_size_frames(buffer_size_t new_buffer_size)
	{
		return false;
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
			// TODO: The magic number 10000000 should be modified to reflect the actual buffer size that we're requesting.
			const int periodicity = 0;
			HRESULT hr = _pAudioClient->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
				10000000,
				periodicity,
				_MixFormat,
				nullptr);
			if (FAILED(hr))
				return false;

			// TODO: Create an event and pass it to _pAudioClient->SetEventHandle().  We should create the event before initializing the audio client
			// in case creating the event fails.

			// TODO: If we've connected a callback, then create a thread, set its priority to realtime, and then
			// to a process()/wait() loop in it.

			// TODO: Call the start callback
			_running = true;
		}

		return true;
	}

	bool stop()
	{
		if (_running)
		{
			// TODO: If there is a thread running, then join() it here.
			// TODO: Call the stop callback that was registered in the start() function.
			
			_pAudioClient->Stop();
			_running = false;
		}

		return true;
	}

	bool is_running() const noexcept
	{
		return _running;
	}

	void wait() const
	{
		// TODO: Wait for the event to be signaled
	}

	template <typename _CallbackType>
	void process(_CallbackType&)
	{
		// TODO: Call AudioRenderClient::GetBuffer() or AudioCaptureClient::GetBuffer().  If it succeeds, then
		// call the user callback, then call the appropriate ::ReleaseBuffer().
	}

	constexpr bool has_unprocessed_io() const noexcept
	{
		// TODO: I think that we need to call AudioClient::GetCurrentPadding() to answer this question?
		// If it returns 0, then it's false.  If it returns 1, then it's true.
		return false;
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
				wstring_convert<codecvt_utf8_utf16<wchar_t>> conv;
				_name = conv.to_bytes(PropertyVariant.pwszVal);
			}

			PropVariantClear(&PropertyVariant);
		}
	}

	void _init_audio_client()
	{
		HRESULT hr = _pDevice->Activate(__wasapi_util::IID_IAudioClient, CLSCTX_ALL, nullptr, &_pAudioClient);
		if (FAILED(hr))
			return;

		/*HRESULT render_hr =*/ _pAudioClient->GetService(__wasapi_util::IID_IAudioRenderClient, &_pAudioRenderClient);
		/*HRESULT capture_hr =*/ _pAudioClient->GetService(__wasapi_util::IID_IAudioCaptureClient, &_pAudioCaptureClient);
	}

	void _init_mix_format()
	{
		/*HRESULT hr =*/ _pAudioClient->GetMixFormat(&_MixFormat);
	}

	IMMDevice* _pDevice = nullptr;
	IAudioClient* _pAudioClient = nullptr;
	IAudioCaptureClient* _pAudioCaptureClient = nullptr;
	IAudioRenderClient* _pAudioRenderClient = nullptr;
	wstring _device_id;
	bool _running = false;
	string _name;
	WAVEFORMATEX* _MixFormat;

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
	audio_device_enumerator() = delete;

	static optional<audio_device> get_default_device(bool bOutputDevice)
	{
		__wasapi_util::CCoInitialize ComInitializer;

		IMMDeviceEnumerator* pEnumerator = nullptr;
		__wasapi_util::AutoRelease EnumeratorRelease{ pEnumerator };

		HRESULT hr = CoCreateInstance(
			__wasapi_util::CLSID_MMDeviceEnumerator, nullptr,
			CLSCTX_ALL, __wasapi_util::IID_IMMDeviceEnumerator,
			static_cast<void**>(&pEnumerator));

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
			static_cast<void**>(&pEnumerator));
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

		for (const auto mmdevice : mmdevices) {
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
