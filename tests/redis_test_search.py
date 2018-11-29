#!/usr/bin/env python2

from redisearch import Client, TextField, Query


def test():
    # Creating a client with a given index name
    client = Client('myIndex')

    # Creating the index definition and schema
    client.drop_index()
    client.create_index([TextField('title', weight=5.0), TextField('body')])

    # Indexing a document
    client.add_document('doc1', title='RediSearch', body='Redisearch implements a search engine on top of redis')

    # Simple search
    res = client.search("search engine")

    # the result has the total number of results, and a list of documents
    print res.total  # "1"
    print res.docs[0]

    # Searching with snippets
    # res = client.search("search engine", snippet_sizes={'body': 50})

    # Searching with complex parameters:
    q = Query("search engine").verbatim().no_content().paging(0, 5)
    res = client.search(q)


test()
