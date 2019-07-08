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
	static const CLSID& get_MMDeviceEnumerator_classid()
	{
		static const CLSID MMDeviceEnumerator_class_id = __uuidof(MMDeviceEnumerator);
		return MMDeviceEnumerator_class_id;
	}
	static const IID& get_IMMDeviceEnumerator_interface_id()
	{
		static const IID IMMDeviceEnumerator_interface_id = __uuidof(IMMDeviceEnumerator);
		return IMMDeviceEnumerator_interface_id;
	}
	static const IID& get_IAudioClient_interface_id()
	{
		static const IID IAudioClient_interface_id = __uuidof(IAudioClient);
		return IAudioClient_interface_id;
	}
	static const IID& get_IAudioRenderClient_interface_id()
	{
		static const IID IAudioRenderClient_interface_id = __uuidof(IAudioRenderClient);
		return IAudioRenderClient_interface_id;
	}
	static const IID& get_IAudioCaptureClient_interface_id()
	{
		static const IID IAudioCaptureClient_interface_id = __uuidof(IAudioCaptureClient);
		return IAudioCaptureClient_interface_id;
	}

	class com_initializer
	{
	public:
		com_initializer() : _hr(CoInitialize(nullptr)) { }
		~com_initializer() { if (SUCCEEDED(_hr)) CoUninitialize(); }
		operator HRESULT() const { return _hr; }
		HRESULT _hr;
	};

	template<typename T>
	class auto_release
	{
	public:
		auto_release(T*& value) :
			_value(value)
		{}

		~auto_release()
		{
			if (_value != nullptr)
				_value->Release();
		}

	private:
		T*& _value;
	};

	static string convert_string(const wchar_t* wide_string)
	{
		int required_characters = WideCharToMultiByte(CP_UTF8, 0, wide_string, -1, nullptr, 0, nullptr, nullptr);
		if (required_characters <= 0)
			return {};

		string output;
		output.resize(static_cast<size_t>(required_characters));
		WideCharToMultiByte(CP_UTF8, 0, wide_string, -1, output.data(), static_cast<int>(output.size()), nullptr, nullptr);
		return output;
	}

	static string convert_string(const wstring& input)
	{
		int required_characters = WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), nullptr, 0, nullptr, nullptr);
		if (required_characters <= 0)
			return {};

		string output;
		output.resize(static_cast<size_t>(required_characters));
		WideCharToMultiByte(CP_UTF8, 0, input.c_str(), static_cast<int>(input.size()), output.data(), static_cast<int>(output.size()), nullptr, nullptr);
		return output;
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
		_device(other._device),
		_audio_client(other._audio_client),
		_audio_capture_client(other._audio_capture_client),
		_audio_render_client(other._audio_render_client),
		_event_handle(other._event_handle),
		_device_id(std::move(other._device_id)),
		_running(other._running.load()),
		_name(std::move(other._name)),
		_mix_format(other._mix_format),
		_processing_thread(std::move(other._processing_thread)),
		_buffer_frame_count(other._buffer_frame_count),
		_is_render_device(other._is_render_device),
		_stop_callback(std::move(other._stop_callback)),
		_user_callback(std::move(other._user_callback))
	{
		other._device = nullptr;
		other._audio_client = nullptr;
		other._audio_capture_client = nullptr;
		other._audio_render_client = nullptr;
		other._event_handle = nullptr;
	}

	audio_device& operator=(audio_device&& other) noexcept
	{
		if (this == &other)
			return *this;

		_device = other._device;
		_audio_client = other._audio_client;
		_audio_capture_client = other._audio_capture_client;
		_audio_render_client = other._audio_render_client;
		_event_handle = other._event_handle;
		_device_id = std::move(other._device_id);
		_running = other._running.load();
		_name = std::move(other._name);
		_mix_format = other._mix_format;
		_processing_thread = std::move(other._processing_thread);
		_buffer_frame_count = other._buffer_frame_count;
		_is_render_device = other._is_render_device;
		_stop_callback = std::move(other._stop_callback);
		_user_callback = std::move(other._user_callback);

		other._device = nullptr;
		other._audio_client = nullptr;
		other._audio_capture_client = nullptr;
		other._audio_render_client = nullptr;
		other._event_handle = nullptr;
	}

	~audio_device()
	{
		stop();

		if (_audio_capture_client != nullptr)
			_audio_capture_client->Release();

		if (_audio_render_client != nullptr)
			_audio_render_client->Release();

		if (_audio_client != nullptr)
			_audio_client->Release();

		if (_device != nullptr)
			_device->Release();
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
		return _is_render_device == false;
	}

	bool is_output() const noexcept
	{
		return _is_render_device == true;
	}

	int get_num_input_channels() const noexcept
	{
		if (is_input() == false)
			return 0;

		return _mix_format.Format.nChannels;
	}

	int get_num_output_channels() const noexcept
	{
		if (is_output() == false)
			return 0;

		return _mix_format.Format.nChannels;
	}

	using sample_rate_t = DWORD;

	sample_rate_t get_sample_rate() const noexcept
	{
		return _mix_format.Format.nSamplesPerSec;
	}

	bool set_sample_rate(sample_rate_t new_sample_rate)
	{
		_mix_format.Format.nSamplesPerSec = new_sample_rate;
		_fixup_mix_format();
		return true;
	}

	using buffer_size_t = UINT32;

	buffer_size_t get_buffer_size_frames() const noexcept
	{
		return _buffer_frame_count;
	}

	bool set_buffer_size_frames(buffer_size_t new_buffer_size)
	{
		_buffer_frame_count = new_buffer_size;
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
			throw audio_device_exception("Cannot change sample type after connecting a callback.");

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
		if (_audio_client == nullptr)
			return false;

		if (!_running)
		{
			_event_handle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (_event_handle == nullptr)
				return false;

			REFERENCE_TIME periodicity = 0;

			const REFERENCE_TIME ref_times_per_second = 10'000'000;
			REFERENCE_TIME buffer_duration = (ref_times_per_second * _buffer_frame_count) / _mix_format.Format.nSamplesPerSec;
			HRESULT hr = _audio_client->Initialize(
				AUDCLNT_SHAREMODE_SHARED,
				AUDCLNT_STREAMFLAGS_RATEADJUST | AUDCLNT_STREAMFLAGS_EVENTCALLBACK,
				buffer_duration,
				periodicity,
				&_mix_format.Format,
				nullptr);

			// TODO: Deal with AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED return code by resetting the buffer_duration and retrying:
			// https://docs.microsoft.com/en-us/windows/desktop/api/audioclient/nf-audioclient-iaudioclient-initialize
			if (FAILED(hr))
				return false;

			/*HRESULT render_hr =*/ _audio_client->GetService(__wasapi_util::get_IAudioRenderClient_interface_id(), reinterpret_cast<void**>(&_audio_render_client));
			/*HRESULT capture_hr =*/ _audio_client->GetService(__wasapi_util::get_IAudioCaptureClient_interface_id(), reinterpret_cast<void**>(&_audio_capture_client));

			// TODO: Make sure to clean up more gracefully from errors
			hr = _audio_client->GetBufferSize(&_buffer_frame_count);
			if (FAILED(hr))
				return false;

			hr = _audio_client->SetEventHandle(_event_handle);
			if (FAILED(hr))
				return false;

			hr = _audio_client->Start();
			if (FAILED(hr))
				return false;

			_running = true;

			if (!_user_callback.valueless_by_exception())
			{
				_processing_thread = thread{ [this]()
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

			if (_processing_thread.joinable())
				_processing_thread.join();

			if (_audio_client != nullptr)
				_audio_client->Stop();
			if (_event_handle != nullptr)
			{
				CloseHandle(_event_handle);
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
		WaitForSingleObject(_event_handle, INFINITE);
	}

	template <typename _CallbackType,
		enable_if_t<is_invocable_v<_CallbackType, audio_device&, audio_device_io<float>&>, int> = 0>
	void process(const _CallbackType& callback)
	{
		if (!_mix_format_matches_type<float>())
			throw audio_device_exception("Attempting to process a callback for a sample type that does not match the configured sample type.");

		_process_helper<float>(callback);
	}

	template <typename _CallbackType,
		enable_if_t<is_invocable_v<_CallbackType, audio_device&, audio_device_io<int32_t>&>, int> = 0>
		void process(const _CallbackType& callback)
	{
		if (!_mix_format_matches_type<int32_t>())
			throw audio_device_exception("Attempting to process a callback for a sample type that does not match the configured sample type.");

		_process_helper<int32_t>(callback);
	}

	template <typename _CallbackType,
		enable_if_t<is_invocable_v<_CallbackType, audio_device&, audio_device_io<int16_t>&>, int> = 0>
		void process(const _CallbackType& callback)
	{
		if (!_mix_format_matches_type<int16_t>())
			throw audio_device_exception("Attempting to process a callback for a sample type that does not match the configured sample type.");

		_process_helper<int16_t>(callback);
	}

	bool has_unprocessed_io() const noexcept
	{
		if (_audio_client == nullptr)
			return false;

		if (!_running)
			return false;

		UINT32 current_padding = 0;
		_audio_client->GetCurrentPadding(&current_padding);
		auto num_frames_available = _buffer_frame_count - current_padding;
		return num_frames_available > 0;
	}

private:
	friend class __audio_device_enumerator;

	audio_device(IMMDevice* device, bool is_render_device) :
		_device(device),
		_is_render_device(is_render_device)
	{
		// TODO: Handle errors better.  Maybe by throwing exceptions?
		if (_device == nullptr)
			throw audio_device_exception("IMMDevice is null.");

		_init_device_id_and_name();
		if (_device_id.empty())
			throw audio_device_exception("Could not get device id.");

		if (_name.empty())
			throw audio_device_exception("Could not get device name.");

		_init_audio_client();
		if (_audio_client == nullptr)
			return;

		_init_mix_format();
	}

	void _init_device_id_and_name()
	{
		LPWSTR device_id = nullptr;
		HRESULT hr = _device->GetId(&device_id);
		if (SUCCEEDED(hr))
		{
			_device_id = device_id;
			CoTaskMemFree(device_id);
		}

		IPropertyStore* property_store = nullptr;
		__wasapi_util::auto_release auto_release_property_store{ property_store };

		hr = _device->OpenPropertyStore(STGM_READ, &property_store);
		if (SUCCEEDED(hr))
		{
			PROPVARIANT property_variant;
			PropVariantInit(&property_variant);

			auto try_acquire_name = [&](const auto& property_name)
			{
				hr = property_store->GetValue(property_name, &property_variant);
				if(SUCCEEDED(hr))
				{
					_name = __wasapi_util::convert_string(property_variant.pwszVal);
					return true;
				}

				return false;
			};

			try_acquire_name(PKEY_Device_FriendlyName)
				|| try_acquire_name(PKEY_DeviceInterface_FriendlyName)
				|| try_acquire_name(PKEY_Device_DeviceDesc);

			PropVariantClear(&property_variant);
		}
	}

	void _init_audio_client()
	{
		HRESULT hr = _device->Activate(__wasapi_util::get_IAudioClient_interface_id(), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&_audio_client));
		if (FAILED(hr))
			return;
	}

	void _init_mix_format()
	{
		WAVEFORMATEX* device_mix_format;
		HRESULT hr = _audio_client->GetMixFormat(&device_mix_format);
		if (FAILED(hr))
			return;

		auto* device_mix_format_ex = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(device_mix_format);
		_mix_format = *device_mix_format_ex;

		CoTaskMemFree(device_mix_format);
	}

	void _fixup_mix_format()
	{
		_mix_format.Format.nBlockAlign = _mix_format.Format.nChannels * _mix_format.Format.wBitsPerSample / 8;
		_mix_format.Format.nAvgBytesPerSec = _mix_format.Format.nSamplesPerSec * _mix_format.Format.wBitsPerSample * _mix_format.Format.nChannels / 8;
	}

	template<typename _CallbackType>
	void _connect_helper(_CallbackType callback)
	{
		if (_running)
			throw audio_device_exception("Cannot connect to running audio_device.");

		_user_callback = move(callback);
	}

	template<typename _SampleType>
	bool _mix_format_matches_type() const noexcept
	{
		if constexpr (is_same_v<_SampleType, float>)
		{
			return _mix_format.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
		}
		else if constexpr (is_same_v<_SampleType, int32_t>)
		{
			return _mix_format.SubFormat == KSDATAFORMAT_SUBTYPE_PCM
				&& _mix_format.Format.wBitsPerSample == sizeof(int32_t) * 8;
		}
		else if constexpr (is_same_v<_SampleType, int16_t>)
		{
			return _mix_format.SubFormat == KSDATAFORMAT_SUBTYPE_PCM
				&& _mix_format.Format.wBitsPerSample == sizeof(int16_t) * 8;
		}
		else
		{
			return false;
		}
	}

	template<typename _SampleType, typename _CallbackType>
	void _process_helper(const _CallbackType& callback)
	{
		if (_audio_client == nullptr)
			return;

		if (!_mix_format_matches_type<_SampleType>())
			return;

		if (is_output())
		{
			UINT32 current_padding = 0;
			_audio_client->GetCurrentPadding(&current_padding);

			auto num_frames_available = _buffer_frame_count - current_padding;
			if (num_frames_available == 0)
				return;

			BYTE* data = nullptr;
			_audio_render_client->GetBuffer(num_frames_available, &data);
			if (data == nullptr)
				return;

			audio_device_io<_SampleType> device_io;
			device_io.output_buffer = { reinterpret_cast<_SampleType*>(data), num_frames_available, _mix_format.Format.nChannels, contiguous_interleaved };
			callback(*this, device_io);

			_audio_render_client->ReleaseBuffer(num_frames_available, 0);
		}
		else if (is_input())
		{
			UINT32 next_packet_size = 0;
			_audio_capture_client->GetNextPacketSize(&next_packet_size);
			if (next_packet_size == 0)
				return;

			// TODO: Support device position.
			DWORD flags = 0;
			BYTE* data = nullptr;
			_audio_capture_client->GetBuffer(&data, &next_packet_size, &flags, nullptr, nullptr);
			if (data == nullptr)
				return;

			audio_device_io<_SampleType> device_io;
			device_io.input_buffer = { reinterpret_cast<_SampleType*>(data), next_packet_size, _mix_format.Format.nChannels, contiguous_interleaved };
			callback(*this, device_io);

			_audio_capture_client->ReleaseBuffer(next_packet_size);
		}
	}

	template <typename _SampleType>
	bool _set_sample_type_helper()
	{
		if constexpr (is_same_v<_SampleType, float>)
		{
			_mix_format.SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
		}
		else if constexpr (is_same_v<_SampleType, int32_t>)
		{
			_mix_format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		}
		else if constexpr (is_same_v<_SampleType, int16_t>)
		{
			_mix_format.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
		}
		else
		{
			return false;
		}
		_mix_format.Format.wBitsPerSample = sizeof(_SampleType) * 8;
		_mix_format.Samples.wValidBitsPerSample = _mix_format.Format.wBitsPerSample;
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

	IMMDevice* _device = nullptr;
	IAudioClient* _audio_client = nullptr;
	IAudioCaptureClient* _audio_capture_client = nullptr;
	IAudioRenderClient* _audio_render_client = nullptr;
	HANDLE _event_handle;
	wstring _device_id;
	atomic<bool> _running = false;
	string _name;

	WAVEFORMATEXTENSIBLE _mix_format;
	thread _processing_thread;
	UINT32 _buffer_frame_count = 0;
	bool _is_render_device = true;

	using __stop_callback_t = function<void(audio_device&)>;
	__stop_callback_t _stop_callback;

	using __wasapi_float_callback_t = function<void(audio_device&, audio_device_io<float>&)>;
	using __wasapi_int32_callback_t = function<void(audio_device&, audio_device_io<int32_t>&)>;
	using __wasapi_int16_callback_t = function<void(audio_device&, audio_device_io<int16_t>&)>;
	variant<__wasapi_float_callback_t, __wasapi_int32_callback_t, __wasapi_int16_callback_t> _user_callback;

	__wasapi_util::com_initializer _com_initializer;
};

class audio_device_list : public forward_list<audio_device> {
};

class __audio_device_enumerator {
public:
	static optional<audio_device> get_default_output_device()
	{
		const bool is_output_device = true;
		return get_default_device(is_output_device);
	};

	static optional<audio_device> get_default_input_device()
	{
		const bool is_output_device = false;
		return get_default_device(is_output_device);
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

	static optional<audio_device> get_default_device(bool output_device)
	{
		__wasapi_util::com_initializer com_initializer;

		IMMDeviceEnumerator* enumerator = nullptr;
		__wasapi_util::auto_release enumerator_release{ enumerator };

		HRESULT hr = CoCreateInstance(
			__wasapi_util::get_MMDeviceEnumerator_classid(), nullptr,
			CLSCTX_ALL, __wasapi_util::get_IMMDeviceEnumerator_interface_id(),
			reinterpret_cast<void**>(&enumerator));

		if (FAILED(hr))
			return nullopt;

		IMMDevice* device = nullptr;
		hr = enumerator->GetDefaultAudioEndpoint(output_device ? eRender : eCapture, eConsole, &device);
		if (FAILED(hr))
			return nullopt;

		try
		{
			return audio_device{ device, output_device };
		}
		catch (const audio_device_exception&)
		{
			return nullopt;
		}
	}

	static vector<IMMDevice*> get_devices(bool output_devices)
	{
		__wasapi_util::com_initializer com_initializer;

		IMMDeviceEnumerator* enumerator = nullptr;
		__wasapi_util::auto_release enumerator_release{ enumerator };
		HRESULT hr = CoCreateInstance(
			__wasapi_util::get_MMDeviceEnumerator_classid(), nullptr,
			CLSCTX_ALL, __wasapi_util::get_IMMDeviceEnumerator_interface_id(),
			reinterpret_cast<void**>(&enumerator));
		if (FAILED(hr))
			return {};

		IMMDeviceCollection* device_collection = nullptr;
		__wasapi_util::auto_release collection_release{ device_collection };

		EDataFlow selected_data_flow = output_devices ? eRender : eCapture;
		hr = enumerator->EnumAudioEndpoints(selected_data_flow, DEVICE_STATE_ACTIVE, &device_collection);
		if (FAILED(hr))
			return {};

		UINT device_count = 0;
		hr = device_collection->GetCount(&device_count);
		if (FAILED(hr))
			return {};

		vector<IMMDevice*> devices;
		for (UINT i = 0; i < device_count; i++)
		{
			IMMDevice* device = nullptr;
			hr = device_collection->Item(i, &device);
			if (FAILED(hr))
			{
				if (device != nullptr)
				{
					device->Release();
				}
				continue;
			}

			if (device != nullptr)
				devices.push_back(device);
		}

		return devices;
	}

	static audio_device_list get_device_list(bool output_devices)
	{
		__wasapi_util::com_initializer com_initializer;
		audio_device_list devices;
		const auto mmdevices = get_devices(output_devices);

		for (auto* mmdevice : mmdevices)
		{
			if (mmdevice == nullptr)
				continue;

			try
			{
				devices.push_front(audio_device{ mmdevice, output_devices });
			}
			catch (const audio_device_exception&)
			{
				// TODO: Should I do anything with this exception?
				// My impulse is to leave it alone.  The result of this function
				// should be an array of properly-constructed devices.  If we
				// couldn't create a device, then we shouldn't return it from
				// this function.
			}
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
			_event(event),
			_callback(std::move(callback))
		{
			if (_enumerator == nullptr)
				throw audio_device_exception("Attempting to create a notification client for a null enumerator");

			_enumerator->RegisterEndpointNotificationCallback(this);
		}

		virtual ~WASAPINotificationClient()
		{
			_enumerator->UnregisterEndpointNotificationCallback(this);
		}

		HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, [[maybe_unused]] LPCWSTR device_id)
		{
			if (role != ERole::eConsole)
				return S_OK;

			if (flow == EDataFlow::eRender)
			{
				if (_event != audio_device_list_event::default_output_device_changed)
					return S_OK;
			}
			else if (flow == EDataFlow::eCapture)
			{
				if (_event != audio_device_list_event::default_input_device_changed)
					return S_OK;
			}
			_callback();
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE OnDeviceAdded([[maybe_unused]] LPCWSTR device_id)
		{
			if (_event != audio_device_list_event::device_list_changed)
				return S_OK;
		
			_callback();
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE OnDeviceRemoved([[maybe_unused]] LPCWSTR device_id)
		{
			if (_event != audio_device_list_event::device_list_changed)
				return S_OK;

			_callback();
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE OnDeviceStateChanged([[maybe_unused]] LPCWSTR device_id, [[maybe_unused]] DWORD new_state)
		{
			if (_event != audio_device_list_event::device_list_changed)
				return S_OK;

			_callback();
			return S_OK;
		}
		HRESULT STDMETHODCALLTYPE OnPropertyValueChanged([[maybe_unused]] LPCWSTR device_id, [[maybe_unused]] const PROPERTYKEY key)
		{
			return S_OK;
		}

		HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **requested_interface)
		{
			if (IID_IUnknown == riid)
			{
				*requested_interface = (IUnknown*)this;
			}
			else if (__uuidof(IMMNotificationClient) == riid)
			{
				*requested_interface = (IMMNotificationClient*)this;
			}
			else
			{
				*requested_interface = nullptr;
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
		__wasapi_util::com_initializer _com_initializer;
		IMMDeviceEnumerator* _enumerator;
		audio_device_list_event _event;
		function<void()> _callback;
	};

	__wasapi_util::com_initializer _com_initializer;
	IMMDeviceEnumerator* _enumerator = nullptr;
	array<unique_ptr<WASAPINotificationClient>, 3> _callback_monitors;
};

template <typename F, typename /* = enable_if_t<is_invocable_v<F>> */>
void set_audio_device_list_callback(audio_device_list_event event, F&& callback)
{
	__audio_device_monitor::instance().register_callback(event, std::move(callback));
}
_LIBSTDAUDIO_NAMESPACE_END
