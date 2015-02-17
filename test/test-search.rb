#!/usr/bin/env ruby
# script to test searching
require 'tuitest'

Kernel.system("rm -f cache cache.lock")

Tuitest.init
verifier = Tuitest::Verifier.new("test-search.rb.log", "RESULT-test-search.rb.xml")


if ENV["OFFLINE"] then
	Tuitest.run("../newsbeuter -c cache -C config-search -u urls-tuitest1-offline")
else
	Tuitest.run("../newsbeuter -c cache -C config-search -u urls-tuitest1")
end


Tuitest.wait_until_expected_text(0, 0, "newsbeuter ", 5000)
Tuitest.keypress("r"[0])

Tuitest.wait_until_idle

Tuitest.keypress("/"[0])

Tuitest.keypress("t"[0])
Tuitest.keypress("h"[0])
Tuitest.keypress("i"[0])
Tuitest.keypress("r"[0])
Tuitest.keypress("d"[0])

Tuitest.keypress(10)
Tuitest.wait_until_idle

Tuitest.wait_until_expected_text(0, 0, "newsbeuter ", 5000)
verifier.expect(1, 3, "1 N  Aug 30   212   |RSS 2.0 testbed f|  RSS 2.0 Item 1")
verifier.expect(2, 3, "2 N  Aug 28   170   |RSS 2.0 testbed f|  RSS 2.0 Item 3")

# Read the first entry
Tuitest.keypress(10)
Tuitest.keypress("q"[0])
Tuitest.wait_until_idle
verifier.expect(1, 3, "1    Aug 30   212   |RSS 2.0 testbed f|  RSS 2.0 Item 1")
verifier.expect(2, 3, "2 N  Aug 28   170   |RSS 2.0 testbed f|  RSS 2.0 Item 3")

# Quit and re-run search
Tuitest.keypress("q"[0])

Tuitest.keypress("/"[0])

Tuitest.keypress("t"[0])
Tuitest.keypress("h"[0])
Tuitest.keypress("i"[0])
Tuitest.keypress("r"[0])
Tuitest.keypress("d"[0])

Tuitest.keypress(10)
Tuitest.wait_until_idle

# The first entry must become invisible because it's already read
Tuitest.wait_until_expected_text(0, 0, "newsbeuter ", 5000)
verifier.expect(1, 3, "1    Aug 30   212   |RSS 2.0 testbed f|  RSS 2.0 Item 1")
verifier.expect(2, 3, "2 N  Aug 28   170   |RSS 2.0 testbed f|  RSS 2.0 Item 3")

Tuitest.keypress("q"[0])
Tuitest.keypress("q"[0])

Tuitest.wait_until_idle

Tuitest.close
verifier.finish

Kernel.system("rm -f cache cache.lock")

# EOF
