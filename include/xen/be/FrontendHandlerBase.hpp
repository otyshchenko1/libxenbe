/*
 *  Xen base frontend handler
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Copyright (C) 2016 EPAM Systems Inc.
 */

#ifndef INCLUDE_FRONTENDHANDLERBASE_HPP_
#define INCLUDE_FRONTENDHANDLERBASE_HPP_

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

extern "C" {
#include <xen/io/xenbus.h>
}

#include "RingBufferBase.hpp"
#include "XenEvtchn.hpp"
#include "XenException.hpp"
#include "XenStore.hpp"
#include "Log.hpp"

namespace XenBackend {

/***************************************************************************//**
 * Exception generated by FrontendHandlerBase.
 * @ingroup backend
 ******************************************************************************/
class FrontendHandlerException : public XenException
{
	using XenException::XenException;
};

class BackendBase;
class XenStore;

/***************************************************************************//**
 * Handles connected frontend.
 * The client should create a class inherited from FrontendHandlerBase and
 * implement onBind() method. This method is invoked when the frontend goes to
 * initialized state. The client should read the channel configuration
 * (ref for the ring buffer and port for the event channel), create DataChannel
 * instance and add with addChennel() method.
 * Example of the client frontend handler class:
 * @code{.cpp}
 * class MyFrontend : public XenBackend::FrontendHandlerBase
 * {
 *     using XenBackend::FrontendHandlerBase::FrontendHandlerBase;
 *
 * private:
 *
 *     void onBind(domid_t domId, uint16_t devId)
 *     {
 *         auto port = getXenStore().readInt("/path/to/eventChannel/port");
 *         uint32_t ref = getXenStore().readInt("/path/to/ringBuffer/ref);
 *
 *         RingBufferPtr ringBuffer(new MyRingBuffer(getDomId(), port, ref));
 *
 *         addChannel(port, ringBuffer);
 *     }
 * };
 * @endcode
 * @ingroup backend
 ******************************************************************************/
class FrontendHandlerBase
{
public:
	/**
	 * @param[in] name                optional frontend name
	 * @param[in] backend             reference to the backend instance
	 * @param[in] waitForInitialising after start waits for frontend is
	 *                                initialising
	 * @param[in] domId               frontend domain id
	 * @param[in] id                  frontend instance id
	 */
	FrontendHandlerBase(const std::string& name, BackendBase& backend,
						domid_t domId, uint16_t devId = 0);

	virtual ~FrontendHandlerBase();

	/**
	 * Returns frontend domain id
	 */
	domid_t getDomId() const { return mDomId; }

	/**
	 * Returns frontend device id
	 */
	uint16_t getDevId() const {  return mDevId; }

	/**
	 * Returns frontend xen store base path
	 */
	const std::string& getXsFrontendPath() const { return mXsFrontendPath; }

	/**
	 * Returns reference to the xen store instance accociated with the frontend
	 */
	XenStore& getXenStore() {  return mXenStore; }

	/**
	 * Check if frontend is terminated
	 */
	bool isTerminated();

protected:
	/**
	 * Is called when the frontend goes to the initialized state.
	 * The client should override this method and create data channels when it
	 * is invoked.
	 */
	virtual void onBind() = 0;

	/**
	 * Adds new ring buffer to the frontend handler.
	 * @param[in] ringBuffer the ring buffer instance
	 */
	void addRingBuffer(RingBufferPtr ringBuffer);

	/**
	 * Returns current backend state.
	 */
	xenbus_state getBackendState() const { return mBackendState; }

	/**
	 * Sets backend state.
	 * @param[in] state new state to set
	 */
	void setBackendState(xenbus_state state);

	/**
	 * Called when the frontend state changed to XenbusStateInitialising
	 */
	virtual void onStateInitializing();

	/**
	 * Called when the frontend state changed to XenbusStateInitWait
	 */
	virtual void onStateInitWait();

	/**
	 * Called when the frontend state changed to XenbusStateInitialized
	 */
	virtual void onStateInitialized();

	/**
	 * Called when the frontend state changed to XenbusStateConnected
	 */
	virtual void onStateConnected();

	/**
	 * Called when the frontend state changed to XenbusStateClosing
	 */
	virtual void onStateClosing();

	/**
	 * Called when the frontend state changed to XenbusStateClosed
	 */
	virtual void onStateClosed();

	/**
	 * Called when the frontend state changed to XenbusStateReconfiguring
	 */
	virtual void onStateReconfiguring();

	/**
	 * Called when the frontend state changed to XenbusStateReconfigured
	 */
	virtual void onStateReconfigured();

private:

	typedef void(FrontendHandlerBase::*StateFn)();

	domid_t mDomId;
	uint16_t mDevId;
	BackendBase& mBackend;

	xenbus_state mBackendState;
	xenbus_state mFrontendState;

	XenStore mXenStore;

	std::string mXsBackendPath;
	std::string mXsFrontendPath;

	std::vector<RingBufferPtr> mRingBuffers;

	std::string mLogId;

	mutable std::mutex mMutex;

	Log mLog;

	void run();

	void initXenStorePathes();
	void checkTerminatedChannels();
	void frontendStateChanged(const std::string& path);
	void onFrontendStateChanged(xenbus_state state);
	void onError(const std::exception& e);
};

typedef std::shared_ptr<FrontendHandlerBase> FrontendHandlerPtr;

}

#endif /* INCLUDE_FRONTENDHANDLERBASE_HPP_ */
