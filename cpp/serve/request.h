/*!
 *  Copyright (c) 2023 by Contributors
 * \file serve/request.h
 * \brief Implementation of llm chat.
 */
#ifndef MLC_LLM_SERVE_REQUEST_H_
#define MLC_LLM_SERVE_REQUEST_H_

#include <tvm/runtime/container/array.h>
#include <tvm/runtime/container/string.h>
#include <tvm/runtime/object.h>
#include <tvm/runtime/packed_func.h>

#include "../tokenizers.h"
#include "config.h"
#include "data.h"

namespace mlc {
namespace llm {
namespace serve {

using namespace tvm::runtime;

/****************** Request ******************/

/*!
 * \brief The user submitted text-generation request, which contains
 * a unique request id, a list of multi-modal inputs, a set of
 * generation configuration parameters.
 * \note Request is immutable and can be re-dispatched to another
 * node and restart the request handling on the new one.
 */
class RequestNode : public Object {
 public:
  /*!
   * \brief The unique identifier of the request.
   * Different requests should have different ids.
   */
  String id;
  /*!
   * \brief The user inputs of a request. Input may have multi-modality.
   * \sa data.h
   */
  Array<Data> inputs;
  /*!
   * \brief The equivalent total input sequence length of the request.
   * "-1" means the total input length is unknown due to the existence
   * of untokenized text data.
   */
  int input_total_length = -1;
  /*!
   * \brief The sampling configuration which may contain temperature,
   * top_p, repetition_penalty, max_gen_len, etc.
   */
  GenerationConfig generation_cfg;

  static constexpr const char* _type_key = "mlc.serve.Request";
  static constexpr const bool _type_has_method_sequal_reduce = false;
  static constexpr const bool _type_has_method_shash_reduce = false;
  TVM_DECLARE_BASE_OBJECT_INFO(RequestNode, Object);
};

class Request : public ObjectRef {
 public:
  explicit Request(String id, Array<Data> inputs, GenerationConfig generation_cfg);

  /*!
   * \brief Return a request object with all text data tokenized,
   * and the request ID kept the same as the input one.
   * \param request The request to be tokenized.
   * \param tokenizer The tokenizer to tokenize the input data of the given request.
   * \return The request object whose data are tokenized.
   */
  static Request FromUntokenized(const Request& request, const Tokenizer& tokenizer);

  TVM_DEFINE_OBJECT_REF_METHODS(Request, ObjectRef, RequestNode);
};

/****************** RequestStreamOutput ******************/

/*!
 * \brief The generated delta request output that is streamed back
 * through callback stream function.
 */
class RequestStreamOutputObj : public Object {
 public:
  /*! \brief The id of the request that the function is invoked for. */
  String request_id;
  /*!
   * \brief The new generated tokens since the last callback invocation
   * for the input request.
   */
  TokenData delta_tokens;
  /*!
   * \brief The finish reason of the request when it is finished,
   * of None if the request has not finished yet.
   */
  Optional<String> finish_reason;

  static constexpr const char* _type_key = "mlc.serve.RequestStreamOutput";
  static constexpr const bool _type_has_method_sequal_reduce = false;
  static constexpr const bool _type_has_method_shash_reduce = false;
  TVM_DECLARE_FINAL_OBJECT_INFO(RequestStreamOutputObj, Object);
};

/*!
 * \brief Managed reference to RequestStreamOutputObj.
 * \sa RequestStreamOutputObj
 */
class RequestStreamOutput : public ObjectRef {
 public:
  explicit RequestStreamOutput(String request_id, TokenData delta_tokens,
                               Optional<String> finish_reason);

  TVM_DEFINE_OBJECT_REF_METHODS(RequestStreamOutput, ObjectRef, RequestStreamOutputObj);
};

}  // namespace serve
}  // namespace llm
}  // namespace mlc

#endif  // MLC_LLM_SERVE_REQUEST_H_
