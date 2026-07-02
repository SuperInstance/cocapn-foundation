# activelog-sync

Optional layer. BYO-cloud sync for households of devices and fleets of boats.
Reference implementation targets Cloudflare (Workers + R2 + D1) because a
non-technical fleet can stand it up on a free tier, but the protocol is plain
HTTPS + object storage semantics — any S3-compatible target works.

## Why sync is almost boring (on purpose)

ActiveLog streams are append-only and globally keyed by `(dev, seq)`. Therefore:

- **Upload** = "push my streams' tail since last ack." Resumable at any byte.
- **Download** = "give me streams I don't have, from seq X." Orderless.
- **Merge** = set union. No conflicts exist by construction. A boat offline for
  three weeks syncs exactly like a boat offline for three seconds, just longer.
- **Media** = content-addressed blobs (sha256); dedup is free; partial sync is
  fine (events first, blobs opportunistically, thumbnails before full clips).

The entire "distributed systems" problem was solved in the spec, which is where
it belongs. This repo is mostly auth, indexing, and politeness about bandwidth.

## Fleet model

A fleet = one bucket + an access list. Streams are shared per-type, opt-in:
a captain may share `catch.assertion` + `fix.track` with the fleet but keep
`speech.segment` private. Sharing granularity is event-type × time-range, chosen
on the phone, enforced at upload (unshared events never leave the boat).

## Tiers

- **T0 boats** never touch this repo: day directories move by USB/AirDrop.
- **BYO** (default): fleet's own Cloudflare account; we're not in the loop.
- **Hosted** (later): our convenience deployment — banner, nominal fee to remove.
  Identical protocol, so migration off us is one config change. The exit door
  stays open on purpose; trust is the product.

## D1 index

Events land in R2 as JSONL day objects; a Worker maintains a D1 index
(dev, day, type, count, geo-bounds) so fleet queries ("coho this week, this
area") run without scanning blobs. The index is a cache, always rebuildable
from R2 — R2 is the only truth.
