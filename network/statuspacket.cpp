//Copyright © 2024 Charles Kerr. All rights reserved.

#include "statuspacket.hpp"

#include <algorithm>
#include <stdexcept>

#include "client.hpp"

using namespace std::string_literals ;

//======================================================================
StatusPacket::StatusPacket(): Packet() {
    this->resize(MINSIZE) ;
    this->setPacketID(PacketType::STATUS) ;
    this->setLength(MINSIZE) ;
}
//======================================================================
StatusPacket::StatusPacket(const std::vector<std::uint8_t> &data):Packet(data) {
    
}

//======================================================================
auto StatusPacket::entryCount() const -> std::uint32_t {
    return this->read<std::uint32_t>(ENTRYOFFSET) ;

}

//======================================================================
auto StatusPacket::setEntryCount(std::uint32_t value) -> void {
    this->write(value,ENTRYOFFSET) ;
}

//======================================================================
auto StatusPacket::setEntries(const std::vector<StatusEntry> &entries) -> void {
    auto size = entries.size() * StatusEntry::ENTRYSIZE + MINSIZE ;
    this->resize(size) ;
    this->setEntryCount(entries.size()) ;
    auto offset = ENTRYOFFSET + 4 ;
    for (const auto &entry : entries) {
        std::copy(entry.data().begin(),entry.data().end(),this->data.begin()+offset) ;
        offset += StatusEntry::ENTRYSIZE ;
    }
}
//======================================================================
auto StatusPacket::allEntries() const -> std::vector<StatusEntry> {
    auto rvalue = std::vector<StatusEntry>() ;
    auto count = this->entryCount();
    auto offset = ENTRYOFFSET + 4 ;
    for (std::uint32_t i = 0 ; i < count ; i++) {
        auto data = std::vector<std::uint8_t>(StatusEntry::ENTRYSIZE,0) ;
        std::copy(this->data.begin()+(offset + i * StatusEntry::ENTRYSIZE),this->data.begin()+(offset + i * StatusEntry::ENTRYSIZE)+ StatusEntry::ENTRYSIZE,data.begin());
        auto entry = StatusEntry(data) ;
        rvalue.push_back(entry) ;
    }
    return rvalue ;
}
