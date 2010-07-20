/**
 * Copyright 2010 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: jmaessen@google.com (Jan Maessen)

#ifndef NET_INSTAWEB_REWRITER_PUBLIC_JAVASCRIPT_FILTER_H_
#define NET_INSTAWEB_REWRITER_PUBLIC_JAVASCRIPT_FILTER_H_

#include <vector>

#include "net/instaweb/rewriter/public/rewrite_filter.h"

#include "net/instaweb/htmlparse/public/html_element.h"
#include "net/instaweb/util/public/atom.h"
#include "net/instaweb/util/public/string_util.h"
#include "net/instaweb/util/public/url_async_fetcher.h"

namespace net_instaweb {
class HtmlParse;
class InputResource;
class MessageHandler;
class MetaData;
class OutputResource;
class ResourceManager;
class Writer;

/**
 * Find Javascript elements (either inline scripts or imported js files) and
 * rewrite them.  This can involve any combination of minifaction,
 * concatenation, renaming, reordering, and incrementalization that accomplishes
 * our goals.
 *
 * For the moment we keep it simple and just minify any scripts that we find.
 *
 * Challenges:
 *  * Identifying everywhere js is invoked, in particular event handlers on
 *    elements that might be found in css or in variously-randomly-named
 *    html properties.
 *  * Analysis of eval() contexts.  Actually less hard than the last, assuming
 *    constant strings.  Otherwise hard.
 *  * Figuring out where to re-inject code after analysis.
 *
 * We will probably need to do an end run around the need for js analysis by
 * instrumenting and incrementally loading code, then probably using dynamic
 * feedback to change the runtime instrumentation in future pages as we serve
 * them.
 */
class JavascriptFilter : public RewriteFilter {
 public:
  JavascriptFilter(const StringPiece& path_prefix,
                   HtmlParse* html_parse,
                   ResourceManager* resource_manager);
  virtual ~JavascriptFilter();
  virtual void StartElement(HtmlElement* element);
  virtual void Characters(HtmlCharactersNode* characters);
  virtual void EndElement(HtmlElement* element);
  virtual void Flush();
  virtual void IEDirective(const std::string& directive);
  virtual bool Fetch(OutputResource* output_resource,
                     Writer* writer,
                     const MetaData& request_header,
                     MetaData* response_headers,
                     UrlAsyncFetcher* fetcher,
                     MessageHandler* message_handler,
                     UrlAsyncFetcher::Callback* callback);
 private:
  inline void CompleteScriptInProgress();
  inline void RewriteInlineScript();
  inline void RewriteExternalScript();
  inline InputResource* ScriptAtUrl(const std::string& script_url);
  bool WriteExternalScriptTo(const std::string& script_url,
                             const std::string& script_out,
                             OutputResource* script_dest);

  std::vector<HtmlCharactersNode*> buffer_;
  HtmlParse* html_parse_;
  HtmlElement* script_in_progress_;
  HtmlElement::Attribute* script_src_;
  ResourceManager* resource_manager_;
  // some_missing_scripts indicates that we stopped processing a script and
  // therefore can't assume we know all of the Javascript on a page.
  bool some_missing_scripts_;
  Atom s_script_;
  Atom s_src_;
};

}  // namespace net_instaweb
#endif  // NET_INSTAWEB_REWRITER_PUBLIC_JAVASCRIPT_FILTER_H_
