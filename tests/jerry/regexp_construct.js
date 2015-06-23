// Copyright 2015 Samsung Electronics Co., Ltd.
// Copyright 2015 University of Szeged.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

var r;

r = new RegExp ("a");
assert (r.source == "a");
assert (r.global == false);
assert (r.ignoreCase == false);
assert (r.multiline == false);

r = new RegExp ("a","gim");
assert (r.source == "a");
// assert (r.global == true);
// assert (r.ignoreCase == true);
// assert (r.multiline == true);

// r = RegExp ("a");
// assert (r.source == "a");
// assert (r.global == false);
// assert (r.ignoreCase == false);
// assert (r.multiline == false);

// r = RegExp ("a","gim");
// assert (r.source == "a");
// assert (r.global == true);
// assert (r.ignoreCase == true);
// assert (r.multiline == true);

// r = /a/gim;
// assert (r.source == "a");
// assert (r.global == false);
// assert (r.ignoreCase == false);
// assert (r.multiline == false);

// r = /a/;
// assert (r.source == "a");
// assert (r.global == true);
// assert (r.ignoreCase == true);
// assert (r.multiline == true);
