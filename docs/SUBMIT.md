# SUBMIT — do these now (5 min)

1. GitHub: new public repo `rythmguard` (Apache-2.0 already in repo).
   `cd /Users/varma/Downloads/CODING/rythmguard`
   `gh repo create rythmguard --public --source=. --push`
   (or: create on github.com, then `git remote add origin <url>; git push -u origin main`)
2. Verify public URL shows: README.md, LICENSE, Makefile, qemu/pulseguard.build,
   qemu/run-qnx.sh, docs/QNX_SETUP.md (QNX 8.0, aarch64le, qcc).
3. Hackathon form: category Robotics & Medical (+ bonus Best Use of QNX).
   Link repo. State: simulation only, host sim runs now, QNX target builds with SDP 8.0.
4. Video (3 min, phone is fine): follow docs/DEMO_SCRIPT.md —
   stable wave → CPU storm → kill sensor → fault.log + misses:0.
   Upload unlisted YouTube, paste link in form.
5. Done. If SDP isn't installed before deadline, submit host-sim repo as-is;
   judges score repo + video, QNX target files already committed as proof of intent.
