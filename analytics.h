#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include <algorithm>
#include <functional>

struct AuthorStat
{
    QString name;
    int paperCount = 0;
};

struct KeywordStat
{
    QString word;
    int frequency = 0;
};

template<typename T, typename Compare>
class BinaryHeap
{
public:
    explicit BinaryHeap(Compare compare = Compare()) : m_compare(compare) {}

    void push(const T& value)
    {
        m_data.push_back(value);
        siftUp(m_data.size() - 1);
    }

    T pop()
    {
        T topValue = m_data.front();
        m_data[0] = m_data.back();
        m_data.pop_back();
        if (!m_data.empty()) {
            siftDown(0);
        }
        return topValue;
    }

    const T& top() const
    {
        return m_data.front();
    }

    int size() const
    {
        return static_cast<int>(m_data.size());
    }

    bool empty() const
    {
        return m_data.empty();
    }

    QVector<T> values() const
    {
        return m_data;
    }

private:
    void siftUp(int index)
    {
        while (index > 0) {
            const int parent = (index - 1) / 2;
            if (!m_compare(m_data[index], m_data[parent])) {
                break;
            }
            std::swap(m_data[index], m_data[parent]);
            index = parent;
        }
    }

    void siftDown(int index)
    {
        const int count = static_cast<int>(m_data.size());
        while (true) {
            int best = index;
            const int left = index * 2 + 1;
            const int right = left + 1;
            if (left < count && m_compare(m_data[left], m_data[best])) {
                best = left;
            }
            if (right < count && m_compare(m_data[right], m_data[best])) {
                best = right;
            }
            if (best == index) {
                break;
            }
            std::swap(m_data[index], m_data[best]);
            index = best;
        }
    }

    QVector<T> m_data;
    Compare m_compare;
};

template<typename Key, typename Value, typename Less = std::less<Key>>
class AvlMap
{
public:
    AvlMap() = default;
    ~AvlMap()
    {
        clear(m_root);
    }

    void insertOrAssign(const Key& key, const Value& value)
    {
        bool inserted = false;
        m_root = insertNode(m_root, key, value, inserted);
        if (inserted) {
            ++m_size;
        }
    }

    bool contains(const Key& key) const
    {
        return findNode(m_root, key) != nullptr;
    }

    Value value(const Key& key, const Value& defaultValue = Value()) const
    {
        Node* node = findNode(m_root, key);
        return node ? node->value : defaultValue;
    }

    QVector<QPair<Key, Value>> inOrder() const
    {
        QVector<QPair<Key, Value>> values;
        values.reserve(m_size);
        traverseInOrder(m_root, values);
        return values;
    }

private:
    struct Node {
        Key key;
        Value value;
        int height = 1;
        Node* left = nullptr;
        Node* right = nullptr;
    };

    int height(Node* node) const
    {
        return node ? node->height : 0;
    }

    int balance(Node* node) const
    {
        return node ? height(node->left) - height(node->right) : 0;
    }

    void updateHeight(Node* node)
    {
        node->height = std::max(height(node->left), height(node->right)) + 1;
    }

    Node* rotateRight(Node* node)
    {
        Node* child = node->left;
        node->left = child->right;
        child->right = node;
        updateHeight(node);
        updateHeight(child);
        return child;
    }

    Node* rotateLeft(Node* node)
    {
        Node* child = node->right;
        node->right = child->left;
        child->left = node;
        updateHeight(node);
        updateHeight(child);
        return child;
    }

    Node* rebalance(Node* node)
    {
        updateHeight(node);
        const int factor = balance(node);
        if (factor > 1) {
            if (balance(node->left) < 0) {
                node->left = rotateLeft(node->left);
            }
            return rotateRight(node);
        }
        if (factor < -1) {
            if (balance(node->right) > 0) {
                node->right = rotateRight(node->right);
            }
            return rotateLeft(node);
        }
        return node;
    }

    Node* insertNode(Node* node, const Key& key, const Value& value, bool& inserted)
    {
        if (!node) {
            inserted = true;
            Node* created = new Node;
            created->key = key;
            created->value = value;
            return created;
        }
        if (m_less(key, node->key)) {
            node->left = insertNode(node->left, key, value, inserted);
        } else if (m_less(node->key, key)) {
            node->right = insertNode(node->right, key, value, inserted);
        } else {
            node->value = value;
            return node;
        }
        return rebalance(node);
    }

    Node* findNode(Node* node, const Key& key) const
    {
        if (!node) {
            return nullptr;
        }
        if (m_less(key, node->key)) {
            return findNode(node->left, key);
        }
        if (m_less(node->key, key)) {
            return findNode(node->right, key);
        }
        return node;
    }

    void traverseInOrder(Node* node, QVector<QPair<Key, Value>>& values) const
    {
        if (!node) {
            return;
        }
        traverseInOrder(node->left, values);
        values.push_back(qMakePair(node->key, node->value));
        traverseInOrder(node->right, values);
    }

    void clear(Node* node)
    {
        if (!node) {
            return;
        }
        clear(node->left);
        clear(node->right);
        delete node;
    }

    Node* m_root = nullptr;
    int m_size = 0;
    Less m_less;
};

template<typename Key, typename Less = std::less<Key>>
class AvlSet
{
public:
    void insert(const Key& key)
    {
        m_storage.insertOrAssign(key, true);
    }

    bool contains(const Key& key) const
    {
        return m_storage.contains(key);
    }

private:
    AvlMap<Key, bool, Less> m_storage;
};

class TrieTree
{
public:
    TrieTree();
    ~TrieTree();

    void insert(const QString& word);
    void collect(QVector<KeywordStat>& output) const;

private:
    struct Node {
        QHash<QChar, Node*> children;
        int count = 0;
        bool isWord = false;
    };

    void collectNode(Node* node, QString& path, QVector<KeywordStat>& output) const;
    void deleteNode(Node* node);

    Node* m_root;
};

class AnalyticsService
{
public:
    explicit AnalyticsService(const QString& xmlPath = QString());

    void setXmlPath(const QString& xmlPath);
    QString baseDirectory() const;

    bool ensureAnalytics();
    QString lastError() const;

    QVector<AuthorStat> topAuthors(int limit = 100) const;
    QVector<AuthorStat> searchAuthors(const QString& keyword, int limit = 100) const;
    QList<int> availableYears() const;
    QVector<KeywordStat> keywordsForYear(int year) const;
    QMap<QString, QVector<QPair<int, int>>> keywordTrends(const QList<int>& years, int topPerYear = 10) const;

private:
    QString normalizeBaseDirectory(const QString& rawPath) const;
    QString authorResultFilePath() const;
    QString hotspotResultFilePath() const;
    bool tryLoadFromMemoryCache();
    bool tryLoadFromResultFiles();
    void updateMemoryCache() const;
    void loadStopWords();
    QStringList tokenizeTitle(const QString& title) const;
    void loadAuthorCountsFromRankFile();
    void loadAuthorStats();
    void loadHotspotStats();
    void writeAuthorResultFile(const QVector<AuthorStat>& authors) const;
    void writeHotspotResultFile() const;
    void rebuildAuthorIndexFromTopAuthors();
    QString sourceVersionStamp() const;

    QString m_xmlPath;
    QString m_baseDirectory;
    QString m_lastError;
    bool m_loaded = false;
    bool m_stopWordsReady = false;

    AvlSet<QString> m_stopWords;
    AvlMap<QString, int> m_authorCounts;
    QVector<AuthorStat> m_topAuthors;
    QMap<int, QVector<KeywordStat>> m_hotKeywordsByYear;
};

#endif
