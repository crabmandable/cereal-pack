#ifndef _CEREAL_PACK_SCHEMA_HPP_
#define _CEREAL_PACK_SCHEMA_HPP_

#include <vector>
#include "properties/Property.hpp"
#include <stdint.h>
#include <stddef.h>

namespace cereal_pack {
    class Schema {
        public:
            void reset() {
                for (auto* prop : properties()) {
                    prop->reset();
                }
            }

            size_t serialize(void* buffer) const {
                unsigned int pos = 0;
                for (auto* p : properties()) {
                    pos += p->serialize((uint8_t*)buffer + pos);
                }
                return pos;
            }
            size_t deserialize(const void* buffer) {
                unsigned int pos = 0;
                for (auto* p : properties()) {
                    pos += p->deserialize((uint8_t*)buffer + pos);
                }
                return pos;
            }
            size_t max_serial_length() const {
                unsigned int accum = 0;
                for (auto* p : properties()) {
                    accum += p->max_serial_length();
                }
                return accum;
            }
            size_t serial_length() const {
                unsigned int accum = 0;
                for (auto* p : properties()) {
                    accum += p->serial_length();
                }
                return accum;
            }

            size_t number_of_properties() const {
                return properties().size();
            }

            const Property* property_at(unsigned int index) const {
                return properties().at(index);
            }
        private:
            virtual const std::vector<Property*>& properties() const = 0;
    };
};
#endif //_CEREAL_PACK_SCHEMA_HPP_
