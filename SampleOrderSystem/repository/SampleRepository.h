#pragma once
#include "../model/Sample.h"
#include <vector>
#include <optional>
#include <string>

class SampleRepository {
public:
    explicit SampleRepository(const std::string& filePath);

    void                     add(const Sample& s);
    std::vector<Sample>      findAll() const;
    std::optional<Sample>    findById(const std::string& id) const;
    std::vector<Sample>      findByName(const std::string& keyword) const;
    void                     update(const Sample& s);

private:
    std::string         filePath_;
    std::vector<Sample> data_;

    void load();
    void save();
};
