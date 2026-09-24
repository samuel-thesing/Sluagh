#ifndef SLUAGH_RESORUCEENTRY_H
#define SLUAGH_RESORUCEENTRY_H

#include <cstdint>
#include <string>

class ResourceEntry final {
    friend class RenderGraph;
    friend class RenderGraphBuilder;

public:
    enum class Type {
        Transient,
        Imported
    };

private:
    struct Concept {
        virtual ~Concept() = default;

        virtual void create(void *allocator) = 0;
        virtual void destroy(void *allocator) = 0;

        virtual void preRead(void *context) = 0;
        virtual void preWrite(void *context) = 0;

        virtual std::string toString() const = 0;
    };

    template<typename T>
    struct Model final : Concept {
        const T::Desc &descriptor;
        T resource;

        Model(const T::Desc &descriptor, T &&resource);

        void create(void *allocator) override;
        void destroy(void *allocator) override;

        void preRead(void *context) override;
        void preWrite(void *context) override;

        std::string toString() const override;
    };

public:
    static constexpr uint32_t initial_version{1u};

private:
    const Type type;
    const uint32_t id;
    uint32_t version;
    std::unique_ptr<Concept> concept_ptr;

    PassNode *producer{nullptr};
    PassNode *last{nullptr};

private:
    template<typename T>
    Model<T>* getModel() const;

    template<typename T>
    ResourceEntry(Type type, uint32_t id, const typename T::Desc &descriptor, T &&resource);

public:
    ResourceEntry(const ResourceEntry &) = delete;
    ResourceEntry(ResourceEntry &&) = default;

    std::string toString() const { return concept_ptr->toString(); }

    void create(void *allocator);
    void destroy(void *allocator);

    void preRead(void *context);
    void preWrite(void *context);

    uint32_t getId() const { return id; }
    uint32_t getVersion() const { return version; }
    bool isImported() const { return type == Type::Imported; }
    bool isTransient() const { return type == Type::Transient; }

    template<typename T>
    T& get();

    template<typename T>
    const T::Desc& getDescriptor() const;
};

#include "ResourceEntry.inl"

#endif // SLUAGH_RESORUCEENTRY_H
