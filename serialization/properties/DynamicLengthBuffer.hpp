#ifndef _CEREAL_PACK_DYNAMICLENGTHBUFFER_HPP_
#define _CEREAL_PACK_DYNAMICLENGTHBUFFER_HPP_

#include "Property.hpp"
#include <stdint.h>
#include <vector>
#include <array>
#include <cstring>

namespace cereal_pack {
    template <size_t max_buffer_length>
    class DynamicLengthBuffer: public Property {
        public:
            DynamicLengthBuffer() = default;

            template < template < class ... > class Container, class ... Args >
            DynamicLengthBuffer(const Container<uint8_t, Args...>& data) {
                if (!length_is_valid(data.size())) {
                    throw "Unable to construct buffer, too big";
                }
                m_value.resize(max_buffer_length);
                std::fill(m_value.begin(), m_value.end(), 0);
                memcpy(m_value.data(), data.data(), data.size());
            }

            template <size_t len>
            DynamicLengthBuffer(const std::array<uint8_t, len>& data) {
                static_assert(len <= max_buffer_length, "Length must fit within the predefined buffer size");
                m_value.resize(len);
                std::fill(m_value.begin(), m_value.end(), 0);
                memcpy(m_value.data(), data.data(), data.size());
            }

            virtual void reset() override {
                m_value.clear();
            }

            virtual size_t serialize(void* buffer) const override {
                *(uint32_t*)buffer = m_value.size();
                memcpy((uint8_t*)buffer + sizeof(uint32_t), m_value.data(), m_value.size());
                return serial_length();
            }

            virtual size_t deserialize(const void* buffer) override {
                uint32_t l = *(uint32_t*)buffer;
                if (!length_is_valid(l)) {
                    //TODO real error
                    throw "Unable to deserialize buffer, it exceeds max length";
                }
                resize(l);
                memcpy(m_value.data(), (uint8_t*)buffer + sizeof(uint32_t), l);
                return serial_length();
            }

            virtual size_t max_serial_length() const override {
                return max_buffer_length + sizeof(uint32_t);
            }

            virtual size_t serial_length() const override {
                return sizeof(uint32_t) + m_value.size();
            }

            void set(uint8_t* data, unsigned int length) {
                if (!length_is_valid(length)) {
                    //TODO real error
                    throw "Unable to set buffer, its too big";
                }
                resize(length);
                memcpy(data, m_value.data(), length);
            }

            void set(const DynamicLengthBuffer<max_buffer_length>& other) {
                *this = other;
            }

            void set(std::vector<uint8_t>&& data) {
                if (!length_is_valid(data.size())) {
                    //TODO real error
                    throw "Unable to set buffer, its too big";
                }
                m_value = data;
            }

            const std::vector<uint8_t>& get() const { return m_value; }

            auto size() const -> typename std::vector<uint8_t>::size_type {
                return m_value.size();
            }
        private:
            bool length_is_valid(unsigned int length) const {
                if (length > max_buffer_length) {
                    return false;
                }
                if (length > m_value.max_size()) {
                    return false;
                }
                return true;
            }
            void resize(unsigned int length) {
                if (length != m_value.size()) {
                    m_value.resize(length);
                }
            }
            std::vector<uint8_t> m_value;
    };
};

#endif //_CEREAL_PACK_DYNAMICLENGTHBUFFER_HPP_
