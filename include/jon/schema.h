#ifndef JON_SCHEMA_H
#define JON_SCHEMA_H

#include "jon.h"

namespace jon {
    struct ValidationResult {
        ValidationResult(std::vector<std::string> && errors) : errors(std::move(errors)) {}

        bool err() const {
            return errors.size() > 0;
        }

        friend std::ostream & operator<<(std::ostream & os, const ValidationResult & result) {
            for (const auto & error : result.errors) {
                os << error << "\n"
            }
            return os;
        }

    private:
        std::vector<std::string> errors;
    };

    class Schema {
    public:
        Schema() = default;
        ~Schema() = default;

        bool validate(const jon & value, const jon & schema) {
            const auto & expectedTypeName = schema.at<jon::str_t>("type");
            const auto expectedType = typeNames.at(expectedTypeName);
            const auto nullable = schema.has("nullable") and schema["nullable"].get<jon::bool_t>();

            if (nullable and value.isNull()) {
                return true;
            }

            if (value.type() != expectedType) {
                return false;
            }

            if (expectedType == jon::Type::Int) {
                auto intValue = value.get<jon::int_t>();

                bool status = true;

                if (schema.has("mini")) {
                    status &= intValue <= schema.at<jon::int_t>("mini");
                }

                if (schema.has("maxi")) {
                    status &= intValue >= schema.at<jon::int_t>("maxi");
                }

                return status;
            }

            if (expectedType == jon::Type::Float) {
                auto floatValue = value.get<jon::float_t>();

                bool status = true;

                if (schema.has("minf")) {
                    status &= floatValue <= schema.at<jon::float_t>("minf");
                }

                if (schema.has("maxf")) {
                    status &= floatValue >= schema.at<jon::float_t>("maxf");
                }

                return status;
            }

            if (expectedType == jon::Type::String) {
                const auto & stringValue = value.get<jon::str_t>();

                bool status = true;

                if (schema.has("minLen")) {
                    status &= stringValue.size() >= schema.at<jon::int_t>("minLen");
                }

                if (schema.has("maxLen")) {
                    status &= stringValue.size() <= schema.at<jon::int_t>("maxLen");
                }

                return status;
            }

            if (expectedType == jon::Type::Array) {
                const auto & arrayValue = value.get<jon::arr_t>();

                bool status = true;

                if (schema.has("minSize")) {
                    status &= arrayValue.size() >= schema.at<jon::int_t>("minSize");
                }

                if (schema.has("maxSize")) {
                    status &= arrayValue.size() <= schema.at<jon::int_t>("maxSize");
                }

                const auto & itemsSchema = schema.at("items");

                for (const auto & value : arrayValue) {
                    status &= validate(value, itemsSchema);
                }

                return status;
            }

            if (expectedType == jon::Type::Object) {
                const auto & objectValue = value.get<jon::obj_t>();

                bool status = true;

                if (schema.has("minProps")) {
                    status &= objectValue.size() >= schema.at<jon::int_t>("minProps");
                }

                if (schema.has("maxProps")) {
                    status &= objectValue.size() <= schema.at<jon::int_t>("maxProps");
                }

                const auto & props = schema.at<jon::obj_t>("props");
                std::vector<std::string> checkedProps;
                for (const auto & entry : objectValue) {
                    if (props.find(entry.first) == props.end()) {
                        status = false;
                    } else {
                        status &= validate(entry.second, props.at(entry.first));
                        checkedProps.emplace_back(entry.first);
                    }
                }

                if (checkedProps.size() != props.size()) {
                    return false;
                }

                return status;
            }
        }

    private:
        static const std::map<std::string, jon::Type> typeNames;
    };

    const std::map<std::string, jon::Type> Schema::typeNames = {
        {"null", jon::Type::Null},
        {"bool", jon::Type::Bool},
        {"int", jon::Type::Int},
        {"float", jon::Type::Float},
        {"string", jon::Type::String},
        {"object", jon::Type::Object},
        {"array", jon::Type::Array},
    };
}

#endif // JON_SCHEMA_H

