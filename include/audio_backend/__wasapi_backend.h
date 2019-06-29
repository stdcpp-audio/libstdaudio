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
#include <variant>
#include <array>

_LIBSTDAUDIO_NAMESPACE_BEGIN

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
	}

	~audio_device()
	{
		stop();

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

		return _MixFormat.Format.nChannels;
	}

	int get_num_output_channels() const noexcept
	{
		if (is_output() == false)
			return 0;

		return _MixFormat.Format.nChannels;
	}

	using sample_rate_t = DWORD;

	sample_rate_t get_sample_rate() const noexcept
	{
		return _MixFormat.Format.nSamplesPerSec;
	}

	bool set_sample_rate(sample_rate_t new_sample_rate)
	{
		_MixFormat.Format.nSamplesPerSec = new_sample_rate;
		_fixup_mix_format();
		return true;
	}

	using buffer_size_t = UINT32;

	buffer_size_t get_buffer_size_frames() const noexcept
	{
		return _bufferFrameCount;
	}

	bool set_buffer_size_frames(buffer_size_t new_buffer_size)
	{
		_bufferFrameCount = new_buffer_size;
		return true;
	}

	template <typename _SampleType>
	constexpr bool supports_sample_type() const noexcept
	{
		return 
			is_same_v<_SampleType, float>
			|| is_same_v<_SampleType, int32_t>
			|| is_same_v<_SampleType, int16_t>;
	}

	template <typename _SampleType>
	bool set_sample_type()
	{
		if (_is_connected() && !is_sample_type<_SampleType>())
			throw audio_device_exception("cannot change sample type after connecting a callback");

		return _set_sample_type_helper<_SampleType>();
	}

	template <typename _SampleType>
	bool is_sample_type() const
	{
		return _mix_format_matches_type<_SampleType>();
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
		enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<float>&>, int> = 0>
	void connect(_CallbackType callback)
	{
		_set_sample_type_helper<float>();
		_connect_helper(__wasapi_float_callback_t{callback});
	}

	template <typename _CallbackType,
		enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<int32_t>&>, int> = 0>
	void connect(_CallbackType callback)
	{
		_set_sample_type_helper<int32_t>();
		_connect_helper(__wasapi_int32_callback_t{callback});
	}

	template <typename _CallbackType,
		enable_if_t<is_nothrow_invocable_v<_CallbackType, audio_device&, audio_device_io<int16_t>&>, int> = 0>
	void connect(_CallbackType callback)
	{
		_set_sample_type_helper<int16_t>();
		_connect_helper(__wasapi_int16_callback_t{ callback });
	}

	// TODO: remove std::function as soon as C++20 default-ctable lambda and lambda in unevaluated contexts become available
	using no_op_t = std::function<void(audio_device&)>;

	template <
		typename _StartCallbackType = no_op_t,
		typename _StopCallbackType = no_op_t,
		// TODO: is_nothrow_invocable_t does not compile, temporarily replaced with is_invocable_t
		typename = enable_if_t<is_invocable_v<_StartCallbackType, audio_device&> && is_invocable_v<_StopCallbackType, audio_device&>>>
	bool start(
		_StartCallbackType&& start_callback = [](audio_device&) noexcept {},
		_StopCallbackType&& stop_callback = [](audio_device&) noexcept {})
	{
		if (_pAudioClient == nullptr)
			return false;

		if (!_running)
		{
			_hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (_hEvent == nullptr)
				return false;

			REFERENCE_TIME periodicity = 0;

			const REFERENCE_TIME RefTimesPerSecond = 10'000'000;
			REFERENCE_TIME buffer_duration = (RefTimesPerSecond * _bufferFrameCount) / _MixFormat.Format.nSamplesPerSec;
			HRESULT hr = _pAudioClient->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
				buffer_duration,
				periodicity,
				&_MixFormat.Format,
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

			if (!_user_callback.valueless_by_exception())
			{
				_ProcessingThread = thread{ [this]()
				{
					SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

					while (_running)
					{
						visit([this](auto&& callback)
							{
								if (callback)
								{
									process(callback);
								}
							},
							_user_callback);
						wait();
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
		enable_if_t<is_invocable_v<_CallbackType, audio_device&, audio_device_io<float>&>, int> = 0>
	void process(const _CallbackType& callback)
	{
		if (!_mix_format_matches_type<float>())
			throw audio_device_exception("attempting to process a callback for a sample type that does not match the configured sample type");

		_process_helper<float>(callback);
	}

	template <typename _CallbackType,
		enable_if_t<is_invocable_v<_CallbackType, audio_device&, audio_device_io<int32_t>&>, int> = 0>
		void process(const _CallbackType& callback)
	{
		if (!_mix_format_matches_type<int32_t>())
			throw audio_device_exception("attempting to process a callback for a sample type that does not match the configured sample type");

		_process_helper<int32_t>(callback);
	}

	template <typename _CallbackType,
		enable_if_t<is_invocable_v<_CallbackType, audio_device&, audio_device_io<int16_t>&>, int> = 0>
		void process(const _CallbackType& callback)
	{
		if (!_mix_format_matches_type<int16_t>())
			throw audio_device_exception("attempting to process a callback for a sample type that does not match the configured sample type");

		_process_helper<int16_t>(callback);
	}

	bool has_unprocessed_io() const noexcept
	{
		if (_pAudioClient == nullptr)
			return false;

		if (!_running)
			return false;

		UINT32 CurrentPadding = 0;
		_pAudioClient->GetCurrentPadding(&CurrentPadding);
		auto NumFramesAvailable = _bufferFrameCount - CurrentPadding;
		return NumFramesAvailable > 0;
	}

private:
	friend class __audio_device_enumerator;

	audio_device(IMMDevice* pDevice, bool bIsRenderDevice) :
		_pDevice(pDevice),
		_IsRenderDevice(bIsRenderDevice)
	{
		// TODO: Handle errors better.  Maybe by throwing exceptions?
		assert(_pDevice != nullptr);

		_init_device_id_and_name();
		assert(!_device_id.empty());
		assert(!_name.empty());

		_init_audio_client();
		if (_pAudioClient == nullptr)
			return;

		_init_mix_format();
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
		WAVEFORMATEX* _DeviceMixFormat;
		HRESULT hr = _pAudioClient->GetMixFormat(&_DeviceMixFormat);
		if (FAILED(hr))
			return;

		auto* _DeviceMixFormatEx = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(_DeviceMixFormat);
		_MixFormat = *_DeviceMixFormatEx;

		CoTaskMemFree(_DeviceMixFormatEx);
	}

	void _fixup_mix_format()
	{
		_MixFormat.Format.nBlockAlign = _MixFormat.Format.nChannels * _MixFormat.Format.wBitsPerSample / 8;
		_MixFormat.Format.nAvgBytesPerSec = _MixFormat.Format.nSamplesPerSec * _MixFormat.Format.wBitsPerSample * _MixFormat.Format.nChannels / 8;
	}

	template<typename _CallbackType>
	void _connect_helper(_CallbackType callback)
	{
		if (_running)
			throw audio_device_exception("cannot connect to running audio_device");

		_user_callback = move(callback);
	}

	template<typename _SampleType>
	bool _mix_format_matches_type() const noexcept
	{
		if constexpr (is_same_v<_SampleType, float>)
		{
			return _MixFormat.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
		}
		else if constexpr (is_same_v<_SampleType, int32_t>)
		{
			return _MixFormat.SubFormat == KSDATAFORMAT_SUBTYPE_PCM
				&& _MixFormat.Format.wBitsPerSample == sizeof(int32_t) * 8;
		}
		else if constexpr (is_same_v<_SampleType, int16_t>)
		{
			return _MixFormat.SubFormat == KSDATAFORMAT_SUBTYPE_PCM
				&& _MixFormat.Format.wBitsPerSample == sizeof(int16_t) * 8;
		}
		else
		{
			return false;
		}
	}

	template<typename _SampleType, typename _CallbackType>
	void _process_helper(const _CallbackType& callback)
	{
		if (_pAudioClient == nullptr)
			return;

		if (!_mix_format_matches_type<_SampleType>())
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

			audio_device_io<_SampleType> device_io;
			device_io.output_buffer = { reinterpret_cast<_SampleType*>(pData), NumFramesAvailable, _MixFormat.Format.nChannels, contiguous_interleaved };
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

			audio_device_io<_SampleType> device_io;
			device_io.input_buffer = { reinterpret_cast<_SampleType*>(pData), NumFrames, _MixFormat.Format.nChannels, contiguous_interleaved };
			callback(*this, device_io);

			_pAudioCaptureClient->ReleaseBuffer(NumFrames);
		}
	}

	template <typename _SampleType>
	bool _set_sample_type_helper()
	{
		if constexpr (is_same_v<_SampleType, float>)
		{
			_MixFormat.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
		}
		else if constexpr (is_same_v<_SampleType, int32_t>)
		{
			_MixFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		}
		else if constexpr (is_same_v<_SampleType, int16_t>)
		{
			_MixFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		}
		else
		{
			return false;
		}
		_MixFormat.Format.wBitsPerSample = sizeof(_SampleType) * 8;
		_MixFormat.Samples.wValidBitsPerSample = _MixFormat.Format.wBitsPerSample;
		_fixup_mix_format();
		return true;
	}

	bool _is_connected() const noexcept
	{
		if (_user_callback.valueless_by_exception())
			return false;

		return visit([](auto&& callback)
		{
			return static_cast<bool>(callback);
		}, _user_callback);
	}

	IMMDevice* _pDevice = nullptr;
	IAudioClient* _pAudioClient = nullptr;
	IAudioCaptureClient* _pAudioCaptureClient = nullptr;
	IAudioRenderClient* _pAudioRenderClient = nullptr;
	HANDLE _hEvent;
	wstring _device_id;
	atomic<bool> _running = false;
	string _name;

	WAVEFORMATEXTENSIBLE _MixFormat;
	thread _ProcessingThread;
	UINT32 _bufferFrameCount = 0;
	bool _IsRenderDevice = true;

	using __stop_callback_t = function<void(audio_device&)>;
	__stop_callback_t _stop_callback;

	using __wasapi_float_callback_t = function<void(audio_device&, audio_device_io<float>&)>;
	using __wasapi_int32_callback_t = function<void(audio_device&, audio_device_io<int32_t>&)>;
	using __wasapi_int16_callback_t = function<void(audio_device&, audio_device_io<int16_t>&)>;
	variant<__wasapi_float_callback_t, __wasapi_int32_callback_t, __wasapi_int16_callback_t> _user_callback;

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
		hr = pEnumerator->EnumAudioEndpoints(SelectedDataFlow, DEVICE_STATE_ACTIVE, &pDeviceCollection);
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

class __audio_device_monitor
{
public:
	static __audio_device_monitor& instance()
	{
		static __audio_device_monitor singleton;
		return singleton;
	}

	template <typename F>
	void register_callback(audio_device_list_event event, F&& callback)
	{
		_callback_monitors[static_cast<int>(event)].reset(new WASAPINotificationClient{_enumerator, event, std::move(callback)});
	}

	template <>
	void register_callback(audio_device_list_event event, nullptr_t&&)
	{
		_callback_monitors[static_cast<int>(event)].reset();
	}

private:
	__audio_device_monitor()
	{
		HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (void**)&_enumerator);
		if (FAILED(hr))
			throw audio_device_exception("Could not create device enumerator");
	}

	~__audio_device_monitor()
	{
		if (_enumerator == nullptr)
			return;

		for (auto& callback_monitor : _callback_monitors)
		{
			callback_monitor.reset();
		}

		_enumerator->Release();
	}

	class WASAPINotificationClient : public IMMNotificationClient
	{
	public:
		WASAPINotificationClient(IMMDeviceEnumerator* enumerator, audio_device_list_event event, function<void()> callback) :
			_enumerator(enumerator),
			event(event),
			callback(std::move(callback))
		{
			if (_enumerator == nullptr)
				throw audio_device_exception("Attempting to create a notification client for a null enumerator");

			_enumerator->RegisterEndpointNotificationCallback(this);
		}

		virtual ~WASAPINotificationClient()
		{
			_enumerator->UnregisterEndpointNotificationCallback(this);
		}

		HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, [[maybe_unused]] LPCWSTR pwstrDeviceId)
		{
			if (role != ERole::eConsole)
				return S_OK;

			if (flow == EDataFlow::eRender)
			{
				if (event != audio_device_list_event::default_output_device_changed)
					return S_OK;
			}
			else if (flow == EDataFlow::eCapture)
			{
				if (event != audio_device_list_event::default_input_device_changed)
					return S_OK;
			}
			callback();
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnDeviceAdded([[maybe_unused]] LPCWSTR pwstrDeviceId)
		{
			if (event != audio_device_list_event::device_list_changed)
				return S_OK;
		
			callback();
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE OnDeviceRemoved([[maybe_unused]] LPCWSTR pwstrDeviceId)
		{
			if (event != audio_device_list_event::device_list_changed)
				return S_OK;

			callback();
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE OnDeviceStateChanged([[maybe_unused]] LPCWSTR pwstrDeviceId, [[maybe_unused]] DWORD dwNewState)
		{
			if (event != audio_device_list_event::device_list_changed)
				return S_OK;

			callback();
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE OnPropertyValueChanged([[maybe_unused]] LPCWSTR pwstrDeviceId, [[maybe_unused]] const PROPERTYKEY key)
		{
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
		{
			if (IID_IUnknown == riid)
			{
				*ppvInterface = (IUnknown*)this;
			}
			else if (__uuidof(IMMNotificationClient) == riid)
			{
				*ppvInterface = (IMMNotificationClient*)this;
			}
			else
			{
				*ppvInterface = nullptr;
				return E_NOINTERFACE;
			}
			return S_OK;
		}

		ULONG STDMETHODCALLTYPE AddRef()
		{
			return 1;
		}

		ULONG STDMETHODCALLTYPE Release()
		{
			return 0;
		}
	
	private:
		__wasapi_util::CCoInitialize ComInitializer;
		IMMDeviceEnumerator* _enumerator;
		audio_device_list_event event;
		function<void()> callback;
	};

	__wasapi_util::CCoInitialize ComInitializer;
	IMMDeviceEnumerator* _enumerator = nullptr;
	array<unique_ptr<WASAPINotificationClient>, 3> _callback_monitors;
};

template <typename F, typename /* = enable_if_t<is_invocable_v<F>> */>
void set_audio_device_list_callback(audio_device_list_event event, F&& callback)
{
	__audio_device_monitor::instance().register_callback(event, std::move(callback));
}
_LIBSTDAUDIO_NAMESPACE_END
