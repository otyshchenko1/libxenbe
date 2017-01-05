/*
 *  Xen Store wrapper
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

#ifndef INCLUDE_XENSTORE_HPP_
#define INCLUDE_XENSTORE_HPP_

#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

extern "C" {
#include <xen/xen.h>
#include <xenstore.h>
}

#include "XenException.hpp"
#include "Log.hpp"

namespace XenBackend {

/***************************************************************************//**
 * Exception generated by XenStore.
 * @ingroup Xen
 ******************************************************************************/
class XenStoreException : public XenException
{
	using XenException::XenException;
};

/***************************************************************************//**
 * Provides Xen Store (XS) functionality.
 * @ingroup Xen
 ******************************************************************************/
class XenStore
{
public:

	/**
	 * Callback which is called when the watch is triggered
	 */
	typedef std::function<void()> WatchCallback;

	/**
	 * @param errorCallback callback called on XS watches error
	 */
	explicit XenStore(ErrorCallback errorCallback = nullptr);
	XenStore(const XenStore&) = delete;
	XenStore& operator=(XenStore const&) = delete;
	~XenStore();

	/**
	 * Returns the home path of the domain.
	 * @param domId domain id
	 */
	std::string getDomainPath(domid_t domId);

	/**
	 * Read XS entry as integer.
	 * @param[in] path path to the entry
	 * @return integer value
	 */
	int readInt(const std::string& path);

	/**
	 * Read XS entry as unsigned integer.
	 * @param[in] path path to the entry
	 * @return integer value
	 */
	unsigned int readUint(const std::string& path);

	/**
	 * Read XS entry as string.
	 * @param[in] path path to the entry
	 * @return string value
	 */
	std::string readString(const std::string& path);

	/**
	 * Writes integer value into XS entry.
	 * @param path  path to the entry
	 * @param value integer value
	 */
	void writeInt(const std::string& path, int value);

	/**
	 * Removes XS entry.
	 * @param path path to the entry
	 */
	void removePath(const std::string& path);

	/**
	 * Checks if XS entry exists.
	 * @param path path to the entry
	 * @return <i>true</i> if the entry exists
	 */
	bool checkIfExist(const std::string& path);

	/**
	 * Reads XS directory
	 * @param path path to the directory
	 * @return string vector of directory items
	 */
	std::vector<std::string> readDirectory(const std::string& path);

	/**
	 * Sets watch for XS entry change.
	 * @param path       path to the entry
	 * @param callback   callback which will be called when the entry is
	 * changed
	 * @param initNotify indicates whether the callback should be called after
	 * adding watch even if there is no changes.
	 */
	void setWatch(const std::string& path, WatchCallback callback,
				  bool initNotify = false);

	/**
	 * Clears watch for XS entry change.
	 * @param path path to the entry.
	 */
	void clearWatch(const std::string& path);

private:

	const int cPollWatchesTimeoutMs = 100;

	ErrorCallback mErrorCallback;

	xs_handle*	mXsHandle;

	std::unordered_map<std::string, WatchCallback> mWatches;
	std::list<std::string> mInitNotifyWatches;

	std::thread mThread;
	std::mutex mMutex;
	std::mutex mItfMutex;
	bool mCheckWatchResult;
	Log mLog;

	void init();
	void release();

	void watchesThread();
	bool isWatchesEmpty();
	std::string checkWatches();
	std::string checkXsWatch();
	bool pollXsWatchFd();
	std::string getInitNotifyPath();
	WatchCallback getWatchCallback(std::string& path);
	void clearWatches();
	void waitWatchesThreadFinished();
};

}

#endif /* INCLUDE_XENSTORE_HPP_ */
