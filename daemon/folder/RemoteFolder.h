/* Copyright (C) 2016 Alexander Shishenko <alex@shishenko.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#pragma once
#include <librevault/Meta.h>
#include <librevault/SignedMeta.h>
#include "AbstractFolder.h"

namespace librevault {

class RemoteFolder : public AbstractFolder, public std::enable_shared_from_this<RemoteFolder> {
	friend class FolderGroup;
public:
	RemoteFolder();
	virtual ~RemoteFolder();

	/* Message senders */
	virtual void choke() = 0;
	virtual void unchoke() = 0;
	virtual void interest() = 0;
	virtual void uninterest() = 0;

	virtual void post_have_meta(const Meta::PathRevision& revision, const bitfield_type& bitfield) = 0;
	virtual void post_have_chunk(const blob& ct_hash) = 0;

	virtual void request_meta(const Meta::PathRevision& revision) = 0;
	virtual void post_meta(const SignedMeta& smeta, const bitfield_type& bitfield) = 0;
	virtual void cancel_meta(const Meta::PathRevision& revision) = 0;

	virtual void request_block(const blob& ct_hash, uint32_t offset, uint32_t size) = 0;
	virtual void post_block(const blob& ct_hash, uint32_t offset, const blob& chunk) = 0;
	virtual void cancel_block(const blob& ct_hash, uint32_t offset, uint32_t size) = 0;

	/* High-level RAII wrappers */
	struct InterestGuard {
		InterestGuard(std::shared_ptr<RemoteFolder> remote);
		~InterestGuard();
	private:
		std::weak_ptr<RemoteFolder> remote_;
	};
	std::shared_ptr<InterestGuard> get_interest_guard();

	/* Getters */
	bool am_choking() const {return am_choking_;}
	bool am_interested() const {return am_interested_;}
	bool peer_choking() const {return peer_choking_;}
	bool peer_interested() const {return peer_interested_;}

	virtual bool ready() const = 0;

protected:
	bool am_choking_ = true;
	bool am_interested_ = false;
	bool peer_choking_ = true;
	bool peer_interested_ = false;

	std::weak_ptr<InterestGuard> interest_guard_;
};

} /* namespace librevault */
