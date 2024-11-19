#pragma once

#include "V8InspectorImpl.h"

namespace PUERTS_NAMESPACE
{
	class V8InspectorEx : public V8Inspector
	{
#if !WITH_QUICKJS
	protected:
		std::vector<std::function<bool(std::weak_ptr<void> handle, const std::string& payload)>> messageListeners;

		std::vector<std::function<void(std::weak_ptr<void> handle)>> openListeners;

		std::vector<std::function<void(std::weak_ptr<void> handle)>> closeListeners;
	public:
		void AddOnOpenListener(std::function<void(std::weak_ptr<void> handle)> listener)
		{
			openListeners.push_back(listener);
		}

		void AddOnCloseListener(std::function<void(std::weak_ptr<void> handle)> listener)
		{
			closeListeners.push_back(listener);
		}

		void AddMessageListener(std::function<bool(std::weak_ptr<void> handle, const std::string& payload)> listener)
		{
			messageListeners.push_back(listener);
		}

		virtual void SendMessagePayload(std::weak_ptr<void> handle, const std::string& payload) = 0;
#endif
	};
} // namespace PUERTS_NAMESPACE