# Vendored Chiaki-ng provenance

This directory vendors Chiaki-ng at commit:

```text
43728af2e639fecb8a7af1cd68c29820a40de973
```

Included dependency commits:

```text
nanopb       160d4f09e5fabb2b66aa2dea32d4f38ace2c4b3f
gf-complete  fa54a4670a5705c84abf6c24b92b0cd479625478
jerasure     505ccb4bc69eee8d7ea40a6089a056b99671134f
```

The upstream license identifiers are retained in the vendored sources. This is a porting workspace: before distributing a combined PS3 package, comply with Chiaki-ng's AGPL-3.0-only-OpenSSL licensing and all nested dependency licenses.

The `ps3/` directory is local port code and does not change upstream protocol/authentication semantics. It deliberately targets local-LAN Remote Play only in its first milestone.
