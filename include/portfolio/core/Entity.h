#ifndef PORTFOLIO_ENTITY_H
#define PORTFOLIO_ENTITY_H

#include "portfolio/core/Date.h"

#include <string>

namespace portfolio {
namespace core {

class Entity {
private:
    long entityId;
    Date createdDate;

public:
    Entity()
        : entityId(0), createdDate() {
    }

    Entity(long id, const Date& date)
        : entityId(id), createdDate(date) {
    }

    virtual ~Entity() {
    }

    long getId() const {
        return entityId;
    }

    Date getCreatedDate() const {
        return createdDate;
    }

    void setEntity(long id, const Date& date) {
        entityId = id;
        createdDate = date;
    }

    virtual std::string describe() const = 0;
};

}
}

#endif
